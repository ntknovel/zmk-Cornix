# Cornix ZMK 한국어 STENO v1.7.0 — canonical exact-mask 약어 수정

## 이번 수정의 핵심

퀵 약어 12개는 정상 출력되지만 기본·접속·어미 약어가 전부 무출력되던 원인을 수정했습니다.
이전 펌웨어에는 완성 사전이 아니라 오래된 **1차/2차 bank 시제품 사전**이 들어 있어,
`그런 = 초ㄱ+종ㄹ`, `그리고 = 초ㄱ+초ㄹ+쌍초`, `였다 = 종ㅇ+종ㄷ` 같은
직접 exact-mask 약어를 찾을 수 없었습니다.

v1.7.0은 완성된 `약어사전파일_v0.1`의 기본 활성 항목 42개를 그대로 사용합니다.

- 기본 약어 17개
- 접속 약어 6개
- 어미 약어 19개
- ZMK Keymap Editor에서 수정 가능한 퀵 Macro 12개

## 선택자 규칙

- `ABBR_L`과 `ABBR_R`은 같은 은행의 좌우 미러가 아니라 **서로 다른 선택자**입니다.
  - 예: `ABBR_L + 종ㅇ + 종ㄷ = 었다`
  - 예: `ABBR_R + 종ㅇ + 종ㄷ = 없다`
- `AB2`만 좌우 VEXT 어느 쪽으로도 입력할 수 있습니다.
  - 예: `VEXT_L/R + 종ㅇ + 종ㄷ = 이었다`
- 선택자가 없는 직접 약어도 정상 작동합니다.
  - `초ㄱ + 종ㄹ = 그런`
  - `초ㄱ + 초ㄹ + 쌍초 = 그리고`
  - `종ㅇ + 종ㄷ = 였다`

## 퀵 약어 12개

기존 `quick_0`~`quick_11` ZMK Macro는 그대로 유지됩니다. 퀵 약어가 먼저 exact-mask로
조회된 뒤 Macro 노드를 호출하므로 Keymap Editor에서 출력 내용만 바꿀 수 있습니다.

## 입력 안정성

- 50ms 일반 key-up 오타 복구 유지
- 선택자 홀드 후 자음 순차 탭 유지
- 완성된 직접 약어 mask는 key-up 보정이 더 짧은 약어로 축약하지 않도록 보호
- 미등록 mask는 유사 단어로 추측하지 않고 무출력

## 적용

저장소 루트에 UPLOAD ZIP을 덮어쓴 뒤:

```bash
python3 tools/validate_package.py .
bash validation/run_host_tests.sh

git add -A
git commit -m "Update Cornix STENO to v1.7.0 canonical dictionary"
git push
```

GitHub Actions에서 left/right/settings_reset 빌드가 모두 성공한 뒤 좌우 UF2를 구분해
플래시합니다. `settings_reset`은 페어링 초기화가 필요할 때만 사용합니다.
