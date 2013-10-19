/**
 * PHP NFC module
 *
 * PHP version 5
 *
 * @author     Nick Silvestro <nick@silvervest.net>
 * @copyright  1997-2005 The PHP Group
 * @license    http://opensource.org/licenses/MIT The MIT License (MIT)
 * @link       https://github.com/silvervest/php_nfc/
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <math.h>
#include "php.h"
#include <nfc/nfc.h>
#include "php_nfc.h"

int le_nfc;
int le_nfc_target;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_nfc_poll, 0, 0, 1)
    ZEND_ARG_INFO(0, nfc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nfc_close, 0, 0, 1)
    ZEND_ARG_INFO(0, nfc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nfc_device_get_name, 0, 0, 1)
    ZEND_ARG_INFO(0, nfc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_nfc_target_get_uid, 0, 0, 1)
    ZEND_ARG_INFO(0, nt)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ nfc_functions[] 
 */
const zend_function_entry nfc_functions[] = {
    PHP_FE(nfc_open,            NULL)
    PHP_FE(nfc_poll,            arginfo_nfc_poll)
    PHP_FE(nfc_close,           arginfo_nfc_close)
    PHP_FE(nfc_device_get_name, arginfo_nfc_device_get_name)
    PHP_FE(nfc_target_get_uid,  arginfo_nfc_target_get_uid)
    PHP_FE_END
};
/* }}} */

/* {{{ nfc_module_entry
 */
zend_module_entry nfc_module_entry = {
    STANDARD_MODULE_HEADER,
    "nfc",
    nfc_functions,
    PHP_MINIT(nfc),
    NULL,
    NULL,
    NULL,
    PHP_MINFO(nfc),
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_NFC
ZEND_GET_MODULE(nfc)
#endif

/* {{{ rsclean
 */
static void rsclean(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    php_nfc *nfc = (php_nfc *)rsrc->ptr;

    nfc_close(nfc->pnd);
    nfc_exit(nfc->context);
    efree(nfc);
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(nfc)
{
    le_nfc = zend_register_list_destructors_ex(rsclean, NULL, "nfc", module_number);
    le_nfc_target = zend_register_list_destructors_ex(NULL, NULL, "nfc", module_number);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(nfc)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "nfc support", "enabled");
    php_info_print_table_end();
}
/* }}} */

/* {{{ proto res nfc_open ()
   opens a connection to and initialises an nfc device */
PHP_FUNCTION(nfc_open)
{
    nfc_connstring *connstring;
    nfc_device *pnd;
    nfc_context *context;
    int rsid;
    php_nfc *nfc;    

    nfc = emalloc(sizeof(php_nfc));
    memset(nfc, 0, sizeof(php_nfc));

    nfc_init(&nfc->context);
    if (nfc->context == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to init libnfc");
        nfc_exit(nfc->context);
        efree(nfc);
        RETURN_FALSE;
    }

    // Open, using the first available NFC device which can be in order of selection:
    //   - default device specified using environment variable or
    //   - first specified device in libnfc.conf (/etc/nfc) or
    //   - first specified device in device-configuration directory (/etc/nfc/devices.d) or
    //   - first auto-detected (if feature is not disabled in libnfc.conf) device
    nfc->pnd = nfc_open(nfc->context, NULL);
    if (nfc->pnd == NULL) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to open NFC device");
        nfc_exit(nfc->context);
        efree(nfc);
        RETURN_FALSE;
    }

    if (nfc_initiator_init(nfc->pnd) < 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to set NFC device to initiator mode");
        nfc_exit(nfc->context);
        efree(nfc);
        RETURN_FALSE;
    }

    ZEND_REGISTER_RESOURCE(return_value, nfc, le_nfc);
}
/* }}} */

/* {{{ proto res nfc_poll (res nfc, int pollNumber, int pollPeriod)
   polls an nfc device for a target */
PHP_FUNCTION(nfc_poll)
{
    zval *nfcr;
    php_nfc *nfc;
    php_nfc_target *nt;
    int res = 0;
    unsigned pollNumber, pollPeriod;

    const size_t szModulations = 5;
    const nfc_modulation nmModulations[5] = {
        { .nmt = NMT_ISO14443A, .nbr = NBR_106 },
        { .nmt = NMT_ISO14443B, .nbr = NBR_106 },
        { .nmt = NMT_FELICA, .nbr = NBR_212 },
        { .nmt = NMT_FELICA, .nbr = NBR_424 },
        { .nmt = NMT_JEWEL, .nbr = NBR_106 },
    };

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|ll", &nfcr, &pollNumber, &pollPeriod) == FAILURE) {
        return;
    }

    if (!pollNumber || pollNumber < 1 || pollNumber > 255) {
        pollNumber = 1;
    }
    if (!pollPeriod || pollPeriod < 1 || pollPeriod > 2250) {
        pollPeriod = 1;
    } else {
        // uiPeriod is 1-16 in increments of 150ms
        // so divice pollPeriod by 150 and round to the nearest int
        pollPeriod = round(pollPeriod / 150);
    }

    ZEND_FETCH_RESOURCE(nfc, php_nfc*, &nfcr, -1, "nfc", le_nfc);

    nt = emalloc(sizeof(php_nfc_target));
    memset(nt, 0, sizeof(php_nfc_target));

    if ((res = nfc_initiator_poll_target(nfc->pnd, nmModulations, szModulations, pollNumber, pollPeriod, &nt->nt))  < 0) {
        efree(nt);
        return;
    }

    if (res > 0) {
        ZEND_REGISTER_RESOURCE(return_value, nt, le_nfc_target);
    } else {
        efree(nt);
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto void nfc_close (res nfc)
   closes an nfc device */
PHP_FUNCTION(nfc_close)
{
    zval *nfcr;
    php_nfc *nfc;
    int type;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &nfcr) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(nfc, php_nfc*, &nfcr, -1, "nfc", le_nfc);

    zend_list_delete(Z_LVAL_P(nfcr));
}
/* }}} */

/* {{{ proto string nfc_target_get_uid (res nt)
   returns the uid of the target */
PHP_FUNCTION(nfc_target_get_uid)
{
    zval *ntr;
    php_nfc_target *nt;
    int type;
    size_t  szPos;
    char *endPtr;
    long val;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &ntr) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(nt, php_nfc_target*, &ntr, -1, "nfc", le_nfc_target);

    char uid[nt->nt.nti.nai.szUidLen];

    for (szPos = 0; szPos < nt->nt.nti.nai.szUidLen; szPos++) {
        sprintf(&uid[szPos*2], "%02x", nt->nt.nti.nai.abtUid[szPos]);
    }
    RETURN_STRING(uid, strlen(uid));
}
/* }}} */

/* {{{ proto string nfc_device_get_name (res nfc)
   gets the reported name of the connected NFC device */
PHP_FUNCTION(nfc_device_get_name)
{
    zval *nfcr;
    php_nfc *nfc;
    const char* deviceName;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &nfcr) == FAILURE) {
        return;
    }

    ZEND_FETCH_RESOURCE(nfc, php_nfc*, &nfcr, -1, "nfc", le_nfc);

    deviceName = nfc_device_get_name(nfc->pnd);
    if (deviceName) {
        RETURN_STRING(deviceName, strlen(deviceName));
    }
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
