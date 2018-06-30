/** @file
  This is a simple shell application

  Copyright (c) 2008 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Pi/PiDxeCis.h>
#include <Protocol/MpService.h>
EFI_MP_SERVICES_PROTOCOL         *mMpService;
extern EFI_SYSTEM_TABLE   *gST;
extern EFI_BOOT_SERVICES  *gBS;
CHAR16                          HexDigit[17] = L"0123456789ABCDEF";

VOID
HexToString (
  CHAR16  *String,
  UINTN   Value,
  UINTN   Digits
  )
{
  for (; Digits > 0; Digits--, String++) {
    *String = HexDigit[((Value >> (4*(Digits-1))) & 0x0f)];
  }
}
/**
  Displays BIST result on Console.

  @param[in] Core         Number of Code
  @param[in] Thread       Number of Code
  @param[in] Eax          Value of Register Eax
**/
VOID
DisplaySelfTestBistResult (
  IN UINT32 Core,
  IN UINT32 Thread,
  IN UINT32 Eax
  )
{
  CHAR16                    String[20];
  CHAR16                    StrBuffer[10];

  gST->ConOut->OutputString (gST->ConOut, L"*******************BIST FAILED*****************\r\n");

  //
  // Display  Core detail
  //
  StrCpyS (String, sizeof (String) / sizeof (CHAR16), L"CORE = 0x");
  HexToString(StrBuffer, Core, 2);
  StrCpyS (String + StrLen(String), sizeof (String) / sizeof (CHAR16) - StrLen (String), StrBuffer);
  gST->ConOut->OutputString (gST->ConOut,String);

  //
  // Display  Thread detail
  //
  StrCpyS (String, sizeof (String) / sizeof (CHAR16), L" Thread = 0x");
  HexToString(StrBuffer, Thread, 2);
  StrCpyS (String + StrLen(String), sizeof (String) / sizeof (CHAR16) - StrLen (String), StrBuffer);
  gST->ConOut->OutputString (gST->ConOut,String);

  //
  // Display  Eax detail
  //
  StrCpyS (String, sizeof (String) / sizeof (CHAR16), L" EAX = 0x");
  HexToString(StrBuffer, Eax, 4);
  StrCpyS (String + StrLen(String), sizeof (String) / sizeof (CHAR16) - StrLen (String), StrBuffer);
  gST->ConOut->OutputString (gST->ConOut,String);
  gST->ConOut->OutputString (gST->ConOut,L"\r\n");
}
/**
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                          Status;
  UINTN 				              Index;
  UINTN 				              NumProc;
  UINTN 				              NumEnabled;
  EFI_PROCESSOR_INFORMATION           MpContext;

  
  Print(L"UefiMain - MP service");
  
  //
  // Locate MP service protocol
  //
  Status =SystemTable->BootServices->LocateProtocol (
                  &gEfiMpServiceProtocolGuid,
                  NULL,
                  (VOID **) &(mMpService)
                  );

  if (EFI_ERROR(Status)) {
	Print(L"Unable to locate the MpService procotol: %r\n", Status);
  }  
  
  // Get Number of Processors and Number of Enabled Processors
  Status = mMpService->GetNumberOfProcessors( mMpService, &NumProc, &NumEnabled);
  if (EFI_ERROR(Status)) {
	Print(L"Unable to get the number of processors: %r\n", Status);
  }else{
	Print(L"NumberOfProcessors: %x\n", NumProc);
	Print(L"NumberOfEnabledProcessors: %x\n", NumEnabled);  
  }
  
  for (Index = 0; Index < NumProc; Index++) {
  Status = mMpService->GetProcessorInfo (mMpService, Index, &MpContext);
  ASSERT_EFI_ERROR (Status);
  if (MpContext.ProcessorId > 255) {
    break;
  }
  if (((MpContext.StatusFlag | PROCESSOR_HEALTH_STATUS_BIT) == 1) || ((MpContext.StatusFlag | PROCESSOR_HEALTH_STATUS_BIT) == 0)) {
    DEBUG ((DEBUG_ERROR, "BIST FAILED CORE=%x THREAD=%x EAX=%x\n", (UINT32)MpContext.Location.Core, (UINT32)MpContext.Location.Thread, MpContext.StatusFlag));
    DisplaySelfTestBistResult(
                 (UINT32)MpContext.Location.Core,
                 (UINT32)MpContext.Location.Thread,
                 MpContext.StatusFlag
                 );
  }
}
  return EFI_SUCCESS;
}
