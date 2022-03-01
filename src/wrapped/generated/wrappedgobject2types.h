/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.1.0.16) *
 *******************************************************************/
#ifndef __wrappedgobject2TYPES_H_
#define __wrappedgobject2TYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef void* (*pFp_t)(void*);
typedef int64_t (*iFpp_t)(void*, void*);
typedef void* (*pFpp_t)(void*, void*);
typedef void (*vFiip_t)(int64_t, int64_t, void*);
typedef void (*vFLLp_t)(uintptr_t, uintptr_t, void*);
typedef void (*vFpup_t)(void*, uint64_t, void*);
typedef void (*vFppV_t)(void*, void*, ...);
typedef void (*vFppA_t)(void*, void*, va_list);
typedef uintptr_t (*LFppp_t)(void*, void*, void*);
typedef void* (*pFLpV_t)(uintptr_t, void*, ...);
typedef void* (*pFLpA_t)(uintptr_t, void*, va_list);
typedef void* (*pFppp_t)(void*, void*, void*);
typedef void* (*pFppV_t)(void*, void*, ...);
typedef void (*vFpuuV_t)(void*, uint64_t, uint64_t, ...);
typedef void (*vFpupp_t)(void*, uint64_t, void*, void*);
typedef void (*vFpupA_t)(void*, uint64_t, void*, va_list);
typedef void (*vFpppp_t)(void*, void*, void*, void*);
typedef uintptr_t (*LFLppi_t)(uintptr_t, void*, void*, int64_t);
typedef uintptr_t (*LFupppp_t)(uint64_t, void*, void*, void*, void*);
typedef uintptr_t (*LFLpppi_t)(uintptr_t, void*, void*, void*, int64_t);
typedef uintptr_t (*LFppppi_t)(void*, void*, void*, void*, int64_t);
typedef uintptr_t (*LFpppppu_t)(void*, void*, void*, void*, void*, uint64_t);
typedef uint64_t (*uFpiupppp_t)(void*, int64_t, uint64_t, void*, void*, void*, void*);
typedef uintptr_t (*LFLpLpLpi_t)(uintptr_t, void*, uintptr_t, void*, uintptr_t, void*, int64_t);
typedef uintptr_t (*LFpiupppp_t)(void*, int64_t, uint64_t, void*, void*, void*, void*);
typedef uint64_t (*uFpLiupppLuV_t)(void*, uintptr_t, int64_t, uint64_t, void*, void*, void*, uintptr_t, uint64_t, ...);
typedef uint64_t (*uFpLippppLup_t)(void*, uintptr_t, int64_t, void*, void*, void*, void*, uintptr_t, uint64_t, void*);
typedef uint64_t (*uFpLippppLuA_t)(void*, uintptr_t, int64_t, void*, void*, void*, void*, uintptr_t, uint64_t, va_list);

#define SUPER() ADDED_FUNCTIONS() \
	GO(g_type_class_peek_parent, pFp_t) \
	GO(g_param_type_register_static, iFpp_t) \
	GO(g_value_array_sort, pFpp_t) \
	GO(g_value_register_transform_func, vFiip_t) \
	GO(g_type_add_interface_static, vFLLp_t) \
	GO(g_object_class_install_properties, vFpup_t) \
	GO(g_object_get, vFppV_t) \
	GO(g_object_set, vFppV_t) \
	GO(g_object_get_valist, vFppA_t) \
	GO(g_object_set_valist, vFppA_t) \
	GO(g_boxed_type_register_static, LFppp_t) \
	GO(g_object_new, pFLpV_t) \
	GO(g_object_new_valist, pFLpA_t) \
	GO(g_value_array_sort_with_data, pFppp_t) \
	GO(g_object_connect, pFppV_t) \
	GO(g_signal_emit, vFpuuV_t) \
	GO(g_param_spec_set_qdata_full, vFpupp_t) \
	GO(g_signal_emit_valist, vFpupA_t) \
	GO(g_object_set_data_full, vFpppp_t) \
	GO(g_object_set_qdata_full, vFpppp_t) \
	GO(g_type_register_static, LFLppi_t) \
	GO(g_signal_add_emission_hook, LFupppp_t) \
	GO(g_type_register_fundamental, LFLpppi_t) \
	GO(g_signal_connect_object, LFppppi_t) \
	GO(g_signal_connect_data, LFpppppu_t) \
	GO(g_signal_handlers_block_matched, uFpiupppp_t) \
	GO(g_signal_handlers_disconnect_matched, uFpiupppp_t) \
	GO(g_signal_handlers_unblock_matched, uFpiupppp_t) \
	GO(g_type_register_static_simple, LFLpLpLpi_t) \
	GO(g_signal_handler_find, LFpiupppp_t) \
	GO(g_signal_new, uFpLiupppLuV_t) \
	GO(g_signal_newv, uFpLippppLup_t) \
	GO(g_signal_new_valist, uFpLippppLuA_t)

#endif // __wrappedgobject2TYPES_H_
