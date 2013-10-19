/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Nick Silvestro <nick@silvervest.net>                        |
   +----------------------------------------------------------------------+
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
