#include "utils.h"

#include <Windows.h>
#include <aclapi.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Disables thread suspension */
void protect_thread(void) {
    const HANDLE curr_thr = GetCurrentThread();

    PACL sacl;
    PSECURITY_DESCRIPTOR descriptor;

    GetSecurityInfo(curr_thr, SE_KERNEL_OBJECT, SACL_SECURITY_INFORMATION, NULL, NULL, NULL, &sacl, &descriptor);
    
    PACL new_sacl;
    EXPLICIT_ACCESS_W access;
    TRUSTEE_W trustee;
    
    trustee.pMultipleTrustee = NULL;
    trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    trustee.TrusteeForm = TRUSTEE_IS_SID;
    trustee.TrusteeType = TRUSTEE_IS_USER;
    trustee.ptstrName = L"CURRENT_USER";

    access.grfAccessMode = SET_ACCESS;
    access.grfAccessPermissions = GENERIC_WRITE | GENERIC_READ;
    access.grfInheritance = NO_INHERITANCE;
    access.Trustee = trustee;

    SetEntriesInAclW(1, &access, sacl, &new_sacl);

    SetSecurityInfo(curr_thr, SE_KERNEL_OBJECT, SACL_SECURITY_INFORMATION, NULL, NULL, NULL, new_sacl);

    LocalFree(descriptor);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */