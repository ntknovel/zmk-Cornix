# Cornix ZMK 한국어 STENO v2.2.2 — 독립 감사 수정판

이 패키지는 Cornix 일반 BASE/RF/좌우 빌드 구조를 유지하면서, 두벌식형 한국어 STENO 입력 엔진을 독립 회귀시험 기준으로 수정한 전체 저장소다.

## 핵심

- 물리 자음 배열: `ㅂㅈㄷㄱㅅ / ㅁㄴㅇㄹㅎ / ㅋㅌㅊㅍ·전용쌍초`, 오른손 종성 대칭
- 오른 엄지: `ㅓ·Space / ㅣ / ㅏ`
- 80ms 단독 자모, 복합모음, 좌우 30개 쌍자음 직접입력
- 기호L/R + QWERTYUIOP 즉시 `1234567890`
- `ㅗ↔ㅜ` Undo, `ㅓ↔ㅏ` Redo 즉시 반복
- 배열 변경 전 canonical 43개 약어 mask 고정, Quick Macro 12개
- raw exact 우선, 쌍자음 alias fallback, ABBR 조건부 L/R fallback
- 전용쌍초/쌍종만 방향 이동; ㅋ/ㅇ alias는 방향키가 아님
- STENO 진입 순간 전체 LED 점등 후 idle 완전 소등, 실제 held key 수만큼 1~4개 점등

전체 실제 입력 규칙은 `FINAL_INPUT_REFERENCE_KO.md`, 변경점은 `V2.2.2_FULL_AUDIT_FIX_KO.txt`를 본다.

## 검증

```bash
python3 tools/validate_package.py .
bash validation/run_host_tests.sh
```

검증에는 42역할 single/pair/triple 12,383건과 별도 입력순서 회귀군이 포함된다.

두 명령 모두 PASS한 뒤 GitHub Actions에서 left/right/settings_reset 빌드를 확인한다. left UF2와 right UF2는 서로 구분해 각 반쪽에 기록한다. settings_reset은 페어링 초기화가 필요할 때만 사용한다.

## 이동

- 약어L + 전용쌍초 탭 → Left
- 약어R + 전용쌍종 탭 → Right
- 기호L + 전용쌍초 탭 → Shift+Left
- 기호R + 전용쌍종 탭 → Shift+Right

canonical 약어 prefix와 겹치는 첫 tap은 보류된다. 뒤에 core가 들어오면 약어가 우선하고, core 없이 약어 selector를 떼면 방향키로 확정한다.
