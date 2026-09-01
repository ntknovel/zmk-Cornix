# Cornix ZMK 한국어 STENO v2.2.6

v2.2.5의 공식 `settings_reset` 빌드 수정과 v2.2.4의 500ms 단독 수정·전역 key-up 삭제 OFF 입력 엔진을 유지하면서, **오른손 종성 자음을 초성으로 빌려 쓰는 피로분산 규칙**과 전용 `,`/`.` 교차 쌍자음 규칙을 추가한 전체 저장소다.

## 새 피로분산 입력

- 초성 없이 정확히 왼손 엄지 `ㅗ`, `ㅡ`, `ㅜ` 중 하나와 오른손 종성 자음 하나를 누르면 오른손 자음을 초성으로 해석한다.
- 오른손 자음은 초성으로 한 번만 출력하며 받침으로 중복하지 않는다.
- 예: `종ㅂ+ㅜ=부`, `종ㄱ+ㅗ=고`, `종ㅁ+ㅡ=므`.
- 전용 쌍초 역할(단독 출력 `,`) + 왼 엄지 모음 + 오른손 `종ㅂ/종ㅈ/종ㄷ/종ㄱ/종ㅅ`은 `ㅃ/ㅉ/ㄸ/ㄲ/ㅆ` 초성을 만든다.
- 전용 쌍종 역할(단독 출력 `.`)도 같은 오른손 미러 초성을 쌍자음으로 만들 수 있다.
- 전용 쌍종 역할 + 왼손 `초ㅂ/초ㅈ/초ㄷ/초ㄱ/초ㅅ` + 모음은 반대편에서 왼손 초성을 쌍자음으로 만든다.
- 요청 예시: `전용쌍초(,)+ㅜ+종ㅂ=뿌`, `전용쌍종(.)+초ㅂ+ㅏ=빠`.
- 새 교차 selector는 전용 `,`/`.` 두 역할만 인정한다. ㅋ/ㅇ·종ㅋ/종ㅇ 별칭은 기존 동일 손 쌍자음 기능만 유지한다.
- 종성→초성 미러는 정확한 `ㅗ/ㅡ/ㅜ` 단모음과 오른손 자음 1개에만 적용한다. 오른손 엄지 모음, 복합모음, 두 종성, 두 전용 selector 동시 입력은 제외한다.
- 왼손 초성이 존재하고 `.` 외에 실제 오른손 자음까지 있으면 기존 종성 쌍자음 해석을 보존한다.

## 기존 입력 동작

- 물리 자음 배열: `ㅂㅈㄷㄱㅅ / ㅁㄴㅇㄹㅎ / ㅋㅌㅊㅍ·전용쌍초`, 오른손 종성 대칭
- 오른 엄지: `ㅓ·Space / ㅣ / ㅏ`; 왼 엄지: `ㅗ / ㅡ / ㅜ·Enter`
- ㅜ/ㅓ 단독 500ms 미만은 Enter/Space, 500ms 이상 hold-release는 직접 ㅜ/ㅓ
- 모든 독립 수정·직접 자모 입력은 500ms 이상 hold-release에서만 실행
- 다른 STENO 역할이 참여하는 순간 단독 후보를 취소하고 정상 chord로 즉시 합류
- 전역 key-up 역할 삭제는 `0ms=OFF`: 먼저 뗀 키도 마지막 key-up까지 stroke에 보존
- 일반 stroke는 마지막 key-up 즉시 확정하며 별도 settle 대기 없음
- 90ms 합법 겹받침 상태 추적은 국소·비차단 방식
- 기호L/R + QWERTYUIOP 즉시 `1234567890`
- `ㅗ↔ㅜ` Undo, `ㅓ↔ㅏ` Redo 즉시 반복
- canonical 약어 43개와 Quick Macro 12개, RF/LED/인코더 구조 유지
- 이동: `약어L + 전용쌍초`=Left, `약어R + 전용쌍종`=Right
- 선택 이동: `기호L + 전용쌍초`=Shift+Left, `기호R + 전용쌍종`=Shift+Right

## 빌드 행

`build.yaml`은 다음 세 산출물을 만든다.

1. `cornix_left_maxrange_steno_indicator`
2. `cornix_right_maxrange_steno_indicator`
3. `cornix_settings_reset`

오른쪽 보드는 BLE peripheral이므로 reset 행은 pinned Cornix 공식 구성과 같은 snippets를 사용한다. 이 `studio-rpc-usb-uart`는 **reset 이미지의 USB 전송 경로용**이며 정상 left STENO 펌웨어에 ZMK Studio를 활성화하는 설정은 아니다.

```yaml
- board: cornix_right//zmk
  shield: settings_reset
  snippet: studio-rpc-usb-uart nrf52840-nosd
  artifact-name: cornix_settings_reset
```

## 검증

```bash
python3 tools/validate_package.py .
bash validation/run_host_tests.sh
```

검증에는 역할 single/pair/triple 12,383건, canonical 약어 172건, 복합모음 38건, 기존 쌍자음 60건, 새 피로분산 release-order 189건과 경계 사례, 500ms 경계, `박` 롤링, OLED가 없는 ZMK 소스 API 형태, 정확한 reset snippets와 commit pin 검사가 포함된다. 실제 nRF52840 cross-build는 GitHub Actions 세 작업이 모두 성공하는지 최종 확인한다.
