# Cornix ZMK 한국어 STENO v1.8.0 — Full Parity

이 버전은 **v1.7.1 canonical 약어·어미 재배치판을 기준**으로, 이전 개발 분기에서 빠졌던
키업·약어·이동·수정·LED 기능과 KLOR에서 최종 확정한 입력 규칙을 한 소스 트리에 다시
통합한 Cornix 최신판이다.

## 1. 보존한 기반

- Cornix 일반 키보드 6개 기존 레이어
- RF 안정화 설정: BLE 1M 우선, +8 dBm, 배터리 보고 300초
- 공식 Bluetooth·배터리 상태 LED
- 좌우 개별 UF2 빌드
- `settings_reset` 별도 산출물
- 최종 완전대칭 STENO 자음 배열
- canonical exact-mask 약어 43개
- GUI 편집형 Quick Macro 12개
- v1.7.1 어미 재배치

## 2. 최종 STENO 자음 배열

```text
                    왼손 초성                         오른손 종성

             Q      W      E      R      T       Y      U      I      O      P
             ㅋ     ㅈ     ㄴ     ㅎ     ㅌ      종ㅌ   종ㅎ   종ㄴ   종ㅈ   종ㅋ

             A      S      D      F      G       H      J      K      L      ;
             ㅅ     ㄱ     ㄹ     ㅇ    쌍초     쌍종   종ㅇ   종ㄹ   종ㄱ   종ㅅ

             Z      X      C      V      B       N      M      ,      .      /
             ㅊ     ㅂ     ㄷ     ㅁ     ㅍ      종ㅍ   종ㅁ   종ㄷ   종ㅂ   종ㅊ
```

## 3. 통일된 좌우 이동

쌍초·쌍종키를 반복 탭키로 통일했다.

| 홀드 | 반복 탭 | 출력 |
|---|---|---|
| 약어L | 쌍초 | `←` 한 칸 |
| 약어R | 쌍종 | `→` 한 칸 |
| 기호L | 쌍초 | `Shift+←` 한 칸 |
| 기호R | 쌍종 | `Shift+→` 한 칸 |

홀드키를 유지한 채 반복 탭할 수 있으며, 이동 뒤 `!`, `"`, `]`, Backspace, `,`, `.` 등의
단독 기능은 추가로 나오지 않는다. 더 큰 정확 약어가 `약어+쌍자음`을 포함하는 경우에는
전체 exact mask가 우선되어 방향키가 약어를 가로채지 않는다.

## 4. 직접 수정 모드

기호L 또는 기호R을 누른 채 자음·모음을 하나씩 탭하면 두벌식 자모를 순서대로 직접 보낸다.
수정 상태에서는 초성·종성 구분 없이 같은 자음으로 정규화한다.

```text
기호 홀드 → 초ㄱ 탭      → ㄱ
기호 홀드 → 종ㄱ 탭      → ㄱ
기호 홀드 → ㄱ → ㅏ → ㄴ → 간
```

쌍자음은 쌍초/쌍종 표식과 자음을 한 단위로 눌러 출력하고, 겹받침은 자음을 차례대로 입력한다.
기호+같은 쪽 쌍자음만 정확히 사용하면 수정 모드가 아니라 Shift 좌우 선택이 된다.

## 5. ㅣ+JLKI와 인코더

- 물리 `ㅣ` 홀드 + I/J/K/L: 위/왼쪽/아래/오른쪽
- Select Nav 레이어의 Shift+I/J/K/L 선택 이동 유지
- 왼쪽 인코더 회전: `Shift+↑ / Shift+↓`
- 오른쪽 인코더 회전: `↑ / ↓`
- 한 노치당 한 번의 press/release

## 6. 영문 복귀 상태

STENO에서 오른쪽 인코더 클릭:

```text
RAlt 1회 → STENO OFF → BASE와 같은 키배치의 영문 상태
```

그 상태에서 오른쪽 인코더를 다시 누르거나 왼쪽 STENO 토글을 누르면:

```text
RAlt 1회 → STENO ON
```

일반 BASE에서 오른쪽 인코더 클릭은 기존 `Ctrl+F`를 유지한다. 별도 동적 레이어를 늘리지 않고
런타임 복귀 상태로 구현했기 때문에 기존 8레이어 키맵과 편집기 구조를 깨지 않는다.

## 7. 약어와 퀵 Macro

- canonical exact-mask 약어: 43개
- GUI 편집형 Quick Macro: 12개
- 약어L과 약어R은 서로 다른 의미
- 일반 AB2는 좌·우 겹모음 어느 쪽으로도 입력 가능
- `였었다`만 오른쪽 겹모음 전용
- 모든 키를 놓은 뒤 exact-mask로만 출력
- 등록되지 않은 조합은 유사 추정 없이 무출력
- 약어 후보인 동안 50ms key-up 보정이 필수 역할을 잘라내지 않음

어미 ㅇㄷ군:

```text
종ㅇ + 종ㄷ                          → 였다
약어L + 종ㅇ + 종ㄷ                 → 었다
약어R + 종ㅇ + 종ㄷ                 → 없다
약어L + 약어R + 종ㅇ + 종ㄷ         → 았다
오른겹모 + 종ㅇ + 종ㄷ              → 였었다
오른쌍종 + 종ㅇ + 종ㄷ              → 이었다
겹모 + 약어R + 종ㅇ + 종ㄷ          → 었었다
겹모 + 약어L + 종ㅇ + 종ㄷ          → 있다
```

Quick Macro의 기본 Text는 `config/steno_quick_text.tsv`에서 수정하고 다음을 실행한다.

```bash
python3 tools/update_quick_macro_text.py
```

## 8. Key-up 보정과 겹받침 롤링

- 일반 오입력 복구: 50ms
- 합법 겹받침 11개 롤링: 90ms
- 먼저 놓인 키가 사전 exact mask의 부분 조합이면 약어 후보로 보존
- 약어/겹모 앵커를 누른 채 자음을 차례로 탭해도 최종 mask 유지
- 팬텀 역할 감사: 42개 역할의 모든 단독·2키·3키, 총 12,383가지 검사
- Cornix는 ZMK 표준 matrix driver를 사용하므로 KLOR의 커스텀 2µs 스캐너 수정은 적용하지 않음

## 9. LED

BASE에서는 공식 Bluetooth·배터리 표시가 LED를 담당한다. STENO가 활성화되면 일시적으로
STENO 피드백이 제어권을 가져간다.

- 진입 플래시
- 현재 물리 홀드 수 1~4개 흰색 표시
- 기본 약어: 초록
- 접속 약어: 노랑
- 어미 약어: 파랑
- STENO 종료 시 공식 상태 표시로 복귀
- Fn-BT의 STENO LED On/Off 유지

## 10. 적용과 빌드

저장소 루트에서 새 브랜치를 만든다.

```bash
cd /workspaces/zmk-Cornix
git status
git switch -c cornix-steno-v1.8.0-full-parity
```

UPLOAD ZIP을 저장소 루트에 덮어쓴 뒤 검증한다.

```bash
unzip -o Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip -d .
rm Cornix_ZMK_STENO_FULL_PARITY_v1.8.0_UPLOAD.zip

python3 tools/validate_package.py .
bash validation/run_host_tests.sh
```

커밋·푸시:

```bash
git add -A
git commit -m "Update Cornix STENO to v1.8.0 full parity"
git push -u origin cornix-steno-v1.8.0-full-parity
```

GitHub Actions에서 다음을 모두 확인한다.

```text
cornix_left_maxrange_steno_indicator
cornix_right_maxrange_steno_indicator
cornix_settings_reset
```

왼쪽과 오른쪽 UF2는 서로 구분해 플래시한다. `settings_reset`은 페어링 초기화가 필요할 때만
사용한다.

## 11. 검증 범위

로컬 패키지 검증은 다음을 실행한다.

```bash
python3 tools/validate_package.py .
bash validation/run_host_tests.sh
```

검사 범위:

- 8레이어 × 50키
- canonical 43개 + Quick Macro 12개
- 좌우 반복 이동과 Shift 선택
- 연속 수정 모드
- ㅣ+JLKI
- 50ms key-up 복구
- 11개 겹받침 90ms 롤링
- 영문 복귀 상태
- LED 카테고리
- 12,383개 팬텀 역할 불변성
- 좌우/peripheral API 형태

전체 Zephyr cross build는 GitHub Actions에서 최종 확인한다.

> `v1.7.x` 이름의 문서는 변경 이력 보존용이다. 현재 적용 기준은 이 README와
> `FULL_PARITY_v1.8.0_KO.md`이다.
