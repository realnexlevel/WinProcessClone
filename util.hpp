#pragma once

class Process
{
public:
	static auto Clone(HANDLE Target, PHANDLE Cloned) -> NTSTATUS
	{
		if (Target == nullptr)
			return STATUS_UNSUCCESSFUL;

		PEPROCESS process = NULL;
		if (!NT_SUCCESS(PsLookupProcessByProcessId(Target, &process)))
		{
			DbgPrint("Failed to open handle to process: %p\n", Target);
			return STATUS_UNSUCCESSFUL;
		}

		DbgPrint("Got process: %p\n", Target);

		HANDLE ProcessHandle = nullptr;
		OBJECT_ATTRIBUTES ObjectAttributes;
		InitializeObjectAttributes(&ObjectAttributes, NULL, 0, NULL, NULL);

		CLIENT_ID CID;
		CID.UniqueProcess = Target;
		CID.UniqueThread = nullptr;
		if (!NT_SUCCESS(ZwOpenProcess(&ProcessHandle, PROCESS_ALL_ACCESS, &ObjectAttributes, &CID)))
		{
			DbgPrint("Failed to open process: %p\n", Target);
			return STATUS_UNSUCCESSFUL;
		}

		DbgPrint("Opened process with handle at addr: %p\n", ProcessHandle);

		HANDLE NewProcHandle = nullptr;
		if (NT_SUCCESS(ZwCreateProcessEx(
			&NewProcHandle,
			PROCESS_ALL_ACCESS,
			NULL,
			ProcessHandle,
			0,
			NULL,
			NULL,
			NULL,
			NULL)))
		{
			*Cloned = NewProcHandle;
			return STATUS_SUCCESS;
		}

		return STATUS_UNSUCCESSFUL;
	}
};