OP-TEE integration for the MS TPM 2.0 Reference Implementation (fTPM)
===========

## fTPM TA
The fTPM Trusted Application (TA) provides a secure firmware implementation
of a TPM using the [MS reference
implementation](https://github.com/microsoft/ms-tpm-20-ref).

The platform specific integration code is kept in this repository.

This is a fork from the the [MS reference
implementation](https://github.com/microsoft/ms-tpm-20-ref) sample
[ARM32-FirmwareTPM](https://github.com/microsoft/ms-tpm-20-ref/tree/Historical_Samples/Samples/ARM32-FirmwareTPM)
maintained to work with OP-TEE.

## Building the TA

This TA is built as described in [Trusted
Applications](https://optee.readthedocs.io/en/latest/building/trusted_applications.html)
section in the [OP-TEE
documentation](https://optee.readthedocs.io/en/latest/index.html) with the
addition of `CFG_MS_TPM_20_REF` which must hold the path to a checked out
copy of the [MS TPM 2.0 Reference
Implementation](https://github.com/microsoft/ms-tpm-20-ref).
You need to checkout commit (98b60a44aba79b15fcce1c0d1e46cf5918400f6a) of [MS TPM 2.0 Reference
Implementation](https://github.com/microsoft/ms-tpm-20-ref)

## Measured Boot support
The fTPM Trusted Application includes support for Measured Boot. This
feature allows the TA to read a TPM Event Log compatible with the
specification in Section 5 of the [TCG EFI Protocol
Specification](https://trustedcomputinggroup.org/wp-content/uploads/EFI-Protocol-Specification-rev13-160330final.pdf).
The event log is read and extended during the TA initialization.

Measure Boot support requires OP-TEE System Call
```PTA_SYSTEM_GET_TPM_EVENT_LOG```.

Flags related to Measured Boot support:

`CFG_TA_MEASURED_BOOT`: Controls whether Measured Boot is enabled
(`CFG_TA_MEASURED_BOOT=y`) or disabled (by default).
`CFG_TA_EVENT_LOG_SIZE`: Maximum size in bytes allowed for the Event Log.
Defaults to 1024 bytes.

