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

#ifndef PHP_NFC_H
#define PHP_NFC_H

extern zend_module_entry nfc_module_entry;
#define phpext_nfc_ptr &nfc_module_entry

PHP_MINIT_FUNCTION(nfc);
PHP_MINFO_FUNCTION(nfc);

PHP_FUNCTION(nfc_open);
PHP_FUNCTION(nfc_poll);
PHP_FUNCTION(nfc_close);
PHP_FUNCTION(nfc_target_get_uid);
PHP_FUNCTION(nfc_device_get_name);

typedef struct _php_nfc
{
    nfc_device *pnd;
    nfc_context *context;
} php_nfc;

typedef struct _php_nfc_target
{
    nfc_target nt;
} php_nfc_target;
#else

#define phpext_nfc_ptr NULL

#endif


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
