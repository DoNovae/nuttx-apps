/**
 * =====================================
 * versions.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */

#include "types.h"

#ifndef __VERSION_H__
#define __VERSION_H__

/**
 *  Release version identifier
 */
#define SHORT_BUILD_VERSION " V1.0"


/**
 * The STRING_DISTRIBUTION_DATE represents when the binary file was built,
 * here we define this default string as the date where the latest release
 * version was tagged.
 */
#define STRING_DISTRIBUTION_DATE "2021-04-01"

/**
 * Defines a generic device name to be output after booting.
 */
#define MACHINE_NAME "ESP32"
#define SOURCE_CODE_URL "www.donovae.com"

/**
 * Default generic device UUID.
 */
#define WEBSITE_URL "www.donovae.com"

#define STRING_CONFIG_H_AUTHOR "DoNovae.com - Herve Bailly"
#define DEFAULT_WEBSITE_URL "www.donovae.com"


#endif //__VERSION_H__
