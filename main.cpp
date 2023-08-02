#include <ntifs.h>
#include <windef.h>

#include "def.hpp"
#include "util.hpp"

NTSTATUS HandleUserRequest(RequestStruct* buffer)
{
	switch (buffer->Type)
	{
	case Request::CloneProcess:
		if (buffer->ProcessId == NULL)
			return STATUS_ABANDONED;

		HANDLE Target = reinterpret_cast<HANDLE>(buffer->ProcessId), Cloned = nullptr;
		if (NT_SUCCESS(Process::Clone(Target, &Cloned)))
		{
			DbgPrint("Created cloned process: %p\n", Cloned);
		}
		else
		{
			DbgPrint("Call to Process::Clone() failed...\n");
		}
		break;
	}

	return STATUS_SUCCESS;
}

NTSTATUS DeviceControlDispatch(PDEVICE_OBJECT DeviceObject, PIRP IRP)
{
	if (DeviceObject == nullptr || IRP == nullptr)
		return STATUS_UNSUCCESSFUL;

	IRP->IoStatus.Status = STATUS_UNSUCCESSFUL;

	auto stack = IoGetCurrentIrpStackLocation(IRP);
	if (stack == nullptr)
		return IRP->IoStatus.Status;

	// Handle our custom incoming requests for cheat related purposes...
	if (stack->Parameters.DeviceIoControl.IoControlCode == CTL_SENDREQUEST)
	{
		auto buffer = reinterpret_cast<PRequestStruct>(IRP->AssociatedIrp.SystemBuffer);
		if (buffer == nullptr)
			return IRP->IoStatus.Status;

		IRP->IoStatus.Status = HandleUserRequest(buffer);
		IRP->IoStatus.Information = sizeof(RequestStruct);

		IoCompleteRequest(IRP, IO_NO_INCREMENT);
		return IRP->IoStatus.Status;
	}

	return IRP->IoStatus.Status;
}

NTSTATUS UnsupportedDispatch(PDEVICE_OBJECT DeviceObject, PIRP IRP)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	IRP->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(IRP, IO_NO_INCREMENT);

	return IRP->IoStatus.Status;
}

NTSTATUS CreateDispatch(PDEVICE_OBJECT DeviceObject, PIRP IRP)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	IoCompleteRequest(IRP, IO_NO_INCREMENT);

	return IRP->IoStatus.Status;
}

NTSTATUS CloseDispatch(PDEVICE_OBJECT DeviceObject, PIRP IRP)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	IoCompleteRequest(IRP, IO_NO_INCREMENT);

	return IRP->IoStatus.Status;
}

NTSTATUS FakeEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	auto Status = STATUS_SUCCESS;
	UNICODE_STRING SymbolicLink, DeviceName;
	PDEVICE_OBJECT DeviceObject;

	RtlInitUnicodeString(&DeviceName, L"\\Device\\CloneInterface");
	Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(Status))
		return Status;

	RtlInitUnicodeString(&SymbolicLink, L"\\DosDevices\\CloneInterface");
	Status = IoCreateSymbolicLink(&SymbolicLink, &DeviceName);
	if (!NT_SUCCESS(Status))
		return Status;

	DeviceObject->Flags |= DO_BUFFERED_IO;

	for (auto t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
		DriverObject->MajorFunction[t] = UnsupportedDispatch;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateDispatch;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseDispatch;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlDispatch;
	DriverObject->DriverUnload = NULL;
	
	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return Status;
}

NTSTATUS DriverEntry()
{
	DbgPrint("DriverEntry called at addr: %p\n", _ReturnAddress());

	// Doing this incase the driver is manual mapped because with most traditional manual mappers they do not pass in a valid driver object...
	return IoCreateDriver(NULL, &FakeEntry);
}