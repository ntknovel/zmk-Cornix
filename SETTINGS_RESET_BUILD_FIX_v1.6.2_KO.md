# Cornix STENO v1.6.2 — 과거 settings_reset 수정 기록 (v2.2.5에서 폐기)

v1.6.2에서는 reset 행의 snippets를 제거했으나, 이후 공개 Actions 재감사에서 그 구성의 `cornix_right//zmk + settings_reset` 작업이 실패하는 것이 확인됐다. 당시 문서의 “공식 예시와 같다”는 판정은 잘못이었다.

v2.2.5부터는 pinned Cornix 공식 하드웨어 모듈과 동일하게 다음을 사용한다.

```yaml
snippet: studio-rpc-usb-uart nrf52840-nosd
```

공유 `config/cornix.conf`에서 STENO를 분리하고 reset에서 `CONFIG_CORNIX_STENO=n`을 유지하는 구조 자체는 계속 유효하다. 현재 적용 기준은 `V2.2.5_SETTINGS_RESET_MATRIX_FIX_KO.md`를 따른다.
