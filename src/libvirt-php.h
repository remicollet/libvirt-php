/*
 * libvirt-php.h: libvirt PHP bindings header file
 *
 * See COPYING for the license of this software
 */

#ifndef PHP_LIBVIRT_H
#define PHP_LIBVIRT_H 1

/* Network constants */
#define VIR_NETWORKS_ACTIVE     1
#define VIR_NETWORKS_INACTIVE       2

/* Version constants */
#define VIR_VERSION_BINDING     1
#define VIR_VERSION_LIBVIRT     2

#ifdef _MSC_VER
#define EXTWIN
#endif

#ifdef EXTWIN
#define COMPILE_DL_LIBVIRT
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef VERSION
#define VERSION "0.5.5"
#define VERSION_MAJOR 0
#define VERSION_MINOR 5
#define VERSION_MICRO 5
#endif

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <libvirt/libvirt-qemu.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <fcntl.h>
#include <sys/types.h>
#include "util.h"

#ifndef EXTWIN
#include <inttypes.h>
#include <dirent.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <stdint.h>
#include <libgen.h>

#else

#define PRIx32       "I32x"
#define PRIx64       "I64x"

#ifdef EXTWIN
#if (_MSC_VER < 1300)
typedef signed char       int8_t;
typedef signed short      int16_t;
typedef signed int        int32_t;
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;
#else
typedef signed __int8     int8_t;
typedef signed __int16    int16_t;
typedef signed __int32    int32_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
#endif
typedef signed __int64       int64_t;
typedef unsigned __int64     uint64_t;
#endif

#endif

#ifdef __i386__
typedef uint32_t arch_uint;
#define UINTx PRIx32
#else
typedef uint64_t arch_uint;
#define UINTx PRIx64
#endif

# define DEBUG_SUPPORT

# ifdef DEBUG_SUPPORT
#  define DEBUG_CORE
#  define DEBUG_VNC
# endif

/* PHP functions are prefixed with `zif_` so strip it */
# define PHPFUNC (__FUNCTION__ + 4)

#ifdef ZTS
#define LIBVIRT_G(v) TSRMG(libvirt_globals_id, zend_libvirt_globals *, v)
#else
#define LIBVIRT_G(v) (libvirt_globals.v)
#endif

#define PHP_LIBVIRT_WORLD_VERSION VERSION
#define PHP_LIBVIRT_WORLD_EXTNAME "libvirt"

/* Internal resource identifier objects */
#define INT_RESOURCE_NODEDEV        0x08
#define INT_RESOURCE_STORAGEPOOL    0x10
#define INT_RESOURCE_VOLUME         0x20
#define INT_RESOURCE_SNAPSHOT       0x40
#define INT_RESOURCE_NWFILTER       0x60

typedef struct tTokenizer {
    char **tokens;
    int numTokens;
} tTokenizer;

typedef struct _resource_info {
    int type;
    virConnectPtr conn;
    void *mem;
    int overwrite;
} resource_info;

typedef struct tVMDisk {
    char *path;
    char *driver;
    char *bus;
    char *dev;
    unsigned long long size;
    int flags;
} tVMDisk;

typedef struct tVMNetwork {
    char *mac;
    char *network;
    char *model;
} tVMNetwork;

/* TODO: temporary forward declarations until other parts are "modularized" */
typedef struct _php_libvirt_connection php_libvirt_connection;
typedef struct _php_libvirt_domain php_libvirt_domain;

/* Libvirt-php types */
typedef struct _php_libvirt_snapshot {
    virDomainSnapshotPtr snapshot;
    php_libvirt_domain* domain;
} php_libvirt_snapshot;

typedef struct _php_libvirt_nodedev {
    virNodeDevicePtr device;
    php_libvirt_connection* conn;
} php_libvirt_nodedev;

typedef struct _php_libvirt_storagepool {
    virStoragePoolPtr pool;
    php_libvirt_connection* conn;
} php_libvirt_storagepool;

typedef struct _php_libvirt_volume {
    virStorageVolPtr volume;
    php_libvirt_connection* conn;
} php_libvirt_volume;

typedef struct _php_libvirt_nwfilter {
    virNWFilterPtr nwfilter;
    php_libvirt_connection* conn;
} php_libvirt_nwfilter;

typedef struct _php_libvirt_hash_key_info {
    char *name;
    unsigned int length;
    unsigned int type;
} php_libvirt_hash_key_info;

ZEND_BEGIN_MODULE_GLOBALS(libvirt)
    char *last_error;
    char *vnc_location;
    zend_bool longlong_to_string_ini;
    char *iso_path_ini;
    char *image_path_ini;
    zend_long max_connections_ini;
# ifdef DEBUG_SUPPORT
    int debug;
# endif
    resource_info *binding_resources;
    int binding_resources_count;
ZEND_END_MODULE_GLOBALS(libvirt)

ZEND_DECLARE_MODULE_GLOBALS(libvirt)

/* Private definitions */
void set_error(char *msg TSRMLS_DC);
void set_error_if_unset(char *msg TSRMLS_DC);
void reset_error(TSRMLS_D);
int count_resources(int type TSRMLS_DC);
int resource_change_counter(int type, virConnectPtr conn, void *mem,
                            int inc TSRMLS_DC);
int check_resource_allocation(virConnectPtr conn, int type,
                              void *mem TSRMLS_DC);
void free_resource(int type, void *mem TSRMLS_DC);
char *connection_get_emulator(virConnectPtr conn, char *arch TSRMLS_DC);
int is_local_connection(virConnectPtr conn);
tTokenizer tokenize(char *string, char *by);
void free_tokens(tTokenizer t);
int set_logfile(char *filename, long maxsize TSRMLS_DC);
char *get_string_from_xpath(char *xml, char *xpath, zval **val, int *retVal);
char **get_array_from_xpath(char *xml, char *xpath, int *num);
void parse_array(zval *arr, tVMDisk *disk, tVMNetwork *network);
char *installation_get_xml(int step, virConnectPtr conn, char *name, int memMB,
                           int maxmemMB, char *arch, char *uuid, int vCpus,
                           char *iso_image, tVMDisk *disks, int numDisks,
                           tVMNetwork *networks, int numNetworks,
                           int domain_flags TSRMLS_DC);
void set_vnc_location(char *msg TSRMLS_DC);
int streamSink(virStreamPtr st ATTRIBUTE_UNUSED,
               const char *bytes, size_t nbytes, void *opaque);
const char *get_feature_binary(const char *name);
long get_next_free_numeric_value(virDomainPtr domain, char *xpath);
int get_subnet_bits(char *ip);

#define PHP_LIBVIRT_STORAGEPOOL_RES_NAME "Libvirt storagepool"
#define PHP_LIBVIRT_VOLUME_RES_NAME "Libvirt volume"
#define PHP_LIBVIRT_NODEDEV_RES_NAME "Libvirt node device"
#define PHP_LIBVIRT_SNAPSHOT_RES_NAME "Libvirt domain snapshot"
#define PHP_LIBVIRT_NWFILTER_RES_NAME "Libvirt nwfilter"

PHP_MINIT_FUNCTION(libvirt);
PHP_MSHUTDOWN_FUNCTION(libvirt);
PHP_RINIT_FUNCTION(libvirt);
PHP_RSHUTDOWN_FUNCTION(libvirt);
PHP_MINFO_FUNCTION(libvirt);

/* Common functions */
PHP_FUNCTION(libvirt_get_last_error);
/* Domain snapshot functions */
PHP_FUNCTION(libvirt_domain_has_current_snapshot);
PHP_FUNCTION(libvirt_domain_snapshot_create);
PHP_FUNCTION(libvirt_domain_snapshot_lookup_by_name);
PHP_FUNCTION(libvirt_domain_snapshot_get_xml);
PHP_FUNCTION(libvirt_domain_snapshot_revert);
PHP_FUNCTION(libvirt_domain_snapshot_delete);
/* Storage pool and storage volume functions */
PHP_FUNCTION(libvirt_storagepool_lookup_by_name);
PHP_FUNCTION(libvirt_storagepool_lookup_by_volume);
PHP_FUNCTION(libvirt_storagepool_list_volumes);
PHP_FUNCTION(libvirt_storagepool_get_info);
PHP_FUNCTION(libvirt_storagevolume_lookup_by_name);
PHP_FUNCTION(libvirt_storagevolume_lookup_by_path);
PHP_FUNCTION(libvirt_storagevolume_get_name);
PHP_FUNCTION(libvirt_storagevolume_get_path);
PHP_FUNCTION(libvirt_storagevolume_get_info);
PHP_FUNCTION(libvirt_storagevolume_get_xml_desc);
PHP_FUNCTION(libvirt_storagevolume_create_xml);
PHP_FUNCTION(libvirt_storagevolume_create_xml_from);
PHP_FUNCTION(libvirt_storagevolume_delete);
PHP_FUNCTION(libvirt_storagevolume_download);
PHP_FUNCTION(libvirt_storagevolume_upload);
PHP_FUNCTION(libvirt_storagevolume_resize);
PHP_FUNCTION(libvirt_storagepool_get_uuid_string);
PHP_FUNCTION(libvirt_storagepool_get_name);
PHP_FUNCTION(libvirt_storagepool_lookup_by_uuid_string);
PHP_FUNCTION(libvirt_storagepool_get_xml_desc);
PHP_FUNCTION(libvirt_storagepool_define_xml);
PHP_FUNCTION(libvirt_storagepool_undefine);
PHP_FUNCTION(libvirt_storagepool_create);
PHP_FUNCTION(libvirt_storagepool_destroy);
PHP_FUNCTION(libvirt_storagepool_is_active);
PHP_FUNCTION(libvirt_storagepool_get_volume_count);
PHP_FUNCTION(libvirt_storagepool_refresh);
PHP_FUNCTION(libvirt_storagepool_set_autostart);
PHP_FUNCTION(libvirt_storagepool_get_autostart);
PHP_FUNCTION(libvirt_storagepool_build);
PHP_FUNCTION(libvirt_storagepool_delete);
/* Nodedev functions */
PHP_FUNCTION(libvirt_nodedev_get);
PHP_FUNCTION(libvirt_nodedev_capabilities);
PHP_FUNCTION(libvirt_nodedev_get_xml_desc);
PHP_FUNCTION(libvirt_nodedev_get_information);
/* NWFilter functions */
PHP_FUNCTION(libvirt_nwfilter_define_xml);
PHP_FUNCTION(libvirt_nwfilter_undefine);
PHP_FUNCTION(libvirt_nwfilter_get_xml_desc);
PHP_FUNCTION(libvirt_nwfilter_get_name);
PHP_FUNCTION(libvirt_nwfilter_get_uuid_string);
PHP_FUNCTION(libvirt_nwfilter_get_uuid);
PHP_FUNCTION(libvirt_nwfilter_lookup_by_name);
PHP_FUNCTION(libvirt_nwfilter_lookup_by_uuid_string);
PHP_FUNCTION(libvirt_nwfilter_lookup_by_uuid);
/* Listing functions */
PHP_FUNCTION(libvirt_list_nodedevs);
PHP_FUNCTION(libvirt_list_all_nwfilters);
PHP_FUNCTION(libvirt_list_nwfilters);
PHP_FUNCTION(libvirt_list_domain_snapshots);
PHP_FUNCTION(libvirt_list_storagepools);
PHP_FUNCTION(libvirt_list_active_storagepools);
PHP_FUNCTION(libvirt_list_inactive_storagepools);
/* Common functions */
PHP_FUNCTION(libvirt_version);
PHP_FUNCTION(libvirt_check_version);
PHP_FUNCTION(libvirt_has_feature);
PHP_FUNCTION(libvirt_get_iso_images);
PHP_FUNCTION(libvirt_image_create);
PHP_FUNCTION(libvirt_image_remove);
/* Debugging functions */
PHP_FUNCTION(libvirt_logfile_set);
PHP_FUNCTION(libvirt_print_binding_resources);

extern zend_module_entry libvirt_module_entry;
#define phpext_libvirt_ptr &libvirt_module_entry

#endif
