#pragma once

#include "types.hpp"

extern "C"
{
    __declspec( dllimport ) PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader( PVOID Base );

    NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
        IN PUNICODE_STRING ObjectName,
        IN ULONG Attributes,
        IN PACCESS_STATE PassedAccessState,
        IN ACCESS_MASK DesiredAccess,
        IN POBJECT_TYPE ObjectType,
        IN KPROCESSOR_MODE AccessMode,
        IN OUT PVOID ParseContext,
        OUT PVOID *ObjectPtr );

    NTSYSAPI
    PVOID
    PsGetProcessSectionBaseAddress(
        __in PEPROCESS Process );

    NTSYSAPI
    PPEB
        NTAPI
        PsGetProcessPeb(
            __in PEPROCESS Process );

    NTSYSAPI
    NTSTATUS
    NTAPI
    MmCopyVirtualMemory(
        IN PEPROCESS FromProcess,
        IN PVOID FromAddress,
        IN PEPROCESS ToProcess,
        OUT PVOID ToAddress,
        IN SIZE_T BufferSize,
        IN KPROCESSOR_MODE PreviousMode,
        OUT PSIZE_T NumberOfBytesCopied );

    NTSYSAPI
    NTSTATUS
    NTAPI
    ZwProtectVirtualMemory(
        __in HANDLE ProcessHandle,
        __inout PVOID *BaseAddress,
        __inout PSIZE_T RegionSize,
        __in ULONG NewProtect,
        __out PULONG OldProtect );

    NTSYSAPI NTSTATUS ZwQuerySystemInformation(
        IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
        _Inout_ PVOID SystemInformation,
        IN ULONG SystemInformationLength,
        _Out_opt_ PULONG ReturnLength );

    NTSYSAPI
    NTSTATUS
    NTAPI
    ZwQueryInformationProcess(
        IN HANDLE ProcessHandle,
        IN PROCESSINFOCLASS ProcessInformationClass,
        OUT PVOID ProcessInformation,
        IN ULONG ProcessInformationLength,
        IN PULONG ReturnLength );

    NTSYSAPI 
        NTSTATUS 
        IoCreateDriver( 
            IN PUNICODE_STRING DriverName, 
            OPTIONAL IN PDRIVER_INITIALIZE InitializationFunction );

    extern POBJECT_TYPE *IoDriverObjectType;
    extern ERESOURCE *PsLoadedModuleResource;
}

namespace imports
{
    using MiGetPteAddress_t = PMMPTE( NTAPI * )( PVOID Base );
    extern MiGetPteAddress_t MiGetPteAddress;

    void init() noexcept;
}