#pragma once

#define CTL_SENDREQUEST	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0999, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

extern "C"
{
	NTSTATUS NTAPI ZwCreateProcessEx(
		_Out_ PHANDLE 	ProcessHandle,
		_In_ ACCESS_MASK 	DesiredAccess,
		_In_opt_ POBJECT_ATTRIBUTES 	ObjectAttributes,
		_In_ HANDLE 	ParentProcess,
		_In_ ULONG 	Flags,
		_In_opt_ HANDLE 	SectionHandle,
		_In_opt_ HANDLE 	DebugPort,
		_In_opt_ HANDLE 	ExceptionPort,
		_In_ ULONG 	JobMemberLevel
	);

	NTSTATUS NTAPI 	IoCreateDriver(
		_In_opt_ PUNICODE_STRING DriverName,
		_In_ PDRIVER_INITIALIZE InitializationFunction
	);

	NTSTATUS NTAPI 	MmCopyVirtualMemory(
		IN PEPROCESS SourceProcess,
		IN PVOID SourceAddress,
		IN PEPROCESS TargetProcess,
		OUT PVOID TargetAddress,
		IN SIZE_T BufferSize,
		IN KPROCESSOR_MODE PreviousMode,
		OUT PSIZE_T ReturnSize
	);
}

enum Request
{
	CloneProcess = 0
};

typedef struct RequestStruct
{
	Request Type;
	DWORD ProcessId;
} RequestStruct, * PRequestStruct;