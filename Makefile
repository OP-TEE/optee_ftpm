BINARY=bc50d971-d4c9-42c4-82cb-343fb7f37896

-include $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk

ifeq ($(wildcard $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk), )
clean:
	@echo 'Note: $$(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk not found, cannot clean TA'
endif

ifeq ($(wildcard $(CFG_MS_TPM_20_REF)),)
ifneq ($(MAKECMDGOALS),clean)
$(error CFG_MS_TPM_20_REF not supplied or invalid)
endif
endif
