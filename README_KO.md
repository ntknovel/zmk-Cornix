# Cornix ZMK 한국어 STENO + 12개 퀵 약어 프리셋 v1.6.3

이 패키지는 v1.5.1의 Cornix STENO·RF·상태 LED·오른손 빌드 하드닝을 유지하면서, 완성 약어사전에 정의된 **12개 퀵 슬롯 M0~M11**을 일반 ZMK Macro로 노출하고 사용자가 지정한 첫 프리셋을 넣은 버전이다.

## 약어 앵커 수정 v1.6.3

실기에서 약어키를 누른 채 자음이나 모음을 차례대로 탭하면, 기존 40ms chord-settle / 50ms key-up correction이 먼저 놓은 역할을 오타로 제거할 수 있었다. 그 결과 완성 약어와 퀵 Macro가 발동하지 않고 약어키 단독 문장부호만 남는 문제가 있었다.

v1.6.3에서는 `약어L/약어R`을 누르고 있는 동안, 그리고 모음이 없는 2차 사전에서 `겹L/겹R`을 누르고 있는 동안 해당 키를 **사전 앵커**로 취급한다. 앵커를 놓을 때까지 중간에 탭한 역할을 exact mask에 보존하므로 다음 입력이 모두 가능하다.

```text
약어L 홀드 → 초ㄱ 탭 → 종ㄹ 탭 → 약어L 해제 = 그런
겹L 홀드   → 초ㄱ 탭 → 종ㄹ 탭 → 겹L 해제   = 그럼
약어L 홀드 → ㅡ 탭 → 약어L 해제             = quick_0
```

추가 키를 잘못 포함하면 비슷한 단어를 추측하지 않고 기존 원칙대로 무출력한다. 일반 음절의 50ms 오타 복구는 그대로 유지된다.


## BASE 레이어 편의키 업데이트 v1.6.2

STENO·약어 기능은 그대로 두고 BASE 레이어의 세 위치를 다음처럼 맞췄다.

| 위치 | 짧은 탭 / 기본 입력 | 홀드 |
|---|---|---|
| 왼쪽 하단 `[` | `[` | `Ctrl+Y` |
| 오른쪽 하단 `]` | `]` | `RCtrl` |
| 오른쪽 최상단 | `Ctrl+A` | 없음 |

실제 ZMK 바인딩은 다음과 같다.

```dts
&ht200 LC(Y) LBKT
&ht200 RCTRL RBKT
&kp LC(A)
```

두 괄호 키는 기존 `ht200` 판정을 사용하므로 200ms 미만의 단독 탭은 괄호, 홀드 또는 다른 키와 조합하면 홀드 동작으로 처리된다.

## 1. 최종 Cornix STENO 배열

```text
[TAB]  ㅋ  ㅈ  ㄴ  ㅎ  ㅌ  |  종ㅌ 종ㅎ 종ㄴ 종ㅈ 종ㅋ  [탭 RAlt / 홀드 RCtrl]
[약L]  ㅅ  ㄱ  ㄹ  ㅇ  쌍초 |  쌍종 종ㅇ 종ㄹ 종ㄱ 종ㅅ  [약R]
[겹L]  ㅊ  ㅂ  ㄷ  ㅁ  ㅍ [ENC-L][ENC-R] 종ㅍ 종ㅁ 종ㄷ 종ㅂ 종ㅊ [겹R]
       LCtrl LAlt 기호L [ㅗ][ㅡ][ㅜ/Enter] | [ㅏ/Space][ㅣ][ㅓ] 기호R Shift BSPC
```

## 2. 완성 약어 사전

- 원본: `config/steno_dictionary.tsv`
- 생성물: `src/steno/steno_dictionary_generated.c`
- 항목 수: 61개
- 모든 참여키를 놓은 마지막 Key-up에서 exact-mask로만 조회
- 모음이 포함된 일반 스트로크는 음절·모음 처리로 이동
- 미등록 자음 조합은 임의 추정하지 않고 무출력
- 출력 뒤 자동 공백·쉼표·마침표·Enter 없음

## 3. 퀵 약어 12개

퀵 입력 조건은 STENO 엔진이 알고 있으므로 Macro 안에 약어키나 모음키를 넣지 않는다. Macro에는 실제 출력할 HID 키 시퀀스만 들어 있다.

| Macro | STENO 입력 | 초기 출력 |
|---|---|---|
| M0 / `quick_0` | 약어L + ㅡ | 사구구 |
| M1 / `quick_1` | 약어L + ㅗ | 구서룡 |
| M2 / `quick_2` | 약어L + ㅜ | 피피쿵 |
| M3 / `quick_3` | 약어R + ㅏ | 스타라이트 |
| M4 / `quick_4` | 약어R + ㅣ | 유안나 |
| M5 / `quick_5` | 약어R + ㅓ | 유혜나 |
| M6 / `quick_6` | 약어L + ㅏ | 실험체 |
| M7 / `quick_7` | 약어L + ㅓ | A 시 |
| M8 / `quick_8` | 약어L + ㅣ | W 시 |
| M9 / `quick_9` | 약어R + ㅗ | 케이지 |
| M10 / `quick_10` | 약어R + ㅜ | 일렉트리스 |
| M11 / `quick_11` | 약어R + ㅡ | 마이스터 |

누르는 순서는 상관없다. 약어키와 해당 모음키가 같은 STENO 스트로크에 포함되고 모두 풀리면 정확히 한 번 실행된다.

## 4. 가장 쉬운 수정 방법

### 방법 A — ZMK Keymap Editor의 Macros 화면

1. 저장소를 ZMK Keymap Editor에서 연다.
2. STENO 레이어에서 Unknown으로 보이는 커스텀 키는 수정하지 않는다.
3. **Macros** 메뉴에서 `quick_0`~`quick_11`을 연다.
4. 출력용 Key Press 시퀀스만 수정한다.
5. Macro 노드 이름 `quick_0`~`quick_11`은 바꾸지 않는다.

### 방법 B — 한국어 텍스트 한 줄 수정

`config/steno_quick_text.tsv`의 세 번째 열만 수정한다.

```text
M0    약어L+ㅡ    사구구
M1    약어L+ㅗ    구서룡
...
```

그다음 실행한다.

```bash
python3 tools/update_quick_macro_text.py
python3 tools/validate_package.py .
```

Windows에서는 저장소 루트의 `UPDATE_QUICK_MACROS.cmd`를 실행할 수 있다. 이 생성기는 완성형 한글을 두벌식 HID로 변환한다. ASCII 영문·숫자 구간은 RAlt로 영문 상태에 들어갔다가 다시 한국어 상태로 돌아오므로 `W 시`, `A 시`처럼 혼합된 출력도 생성할 수 있다.

**주의:** 방법 B를 다시 실행하면 Keymap Editor에서 직접 바꾼 `quick_0`~`quick_11` 본문은 TSV의 현재 값으로 덮어써진다.

## 5. 관련 파일

```text
config/steno_quick_abbreviations.tsv  고정 STENO 입력 마스크 12개
config/steno_quick_text.tsv         텍스트 프리셋 12개
config/cornix.keymap                  Keymap Editor에 보이는 standard ZMK Macro 12개
src/steno/steno_quick_generated.c     입력 마스크 -> M0~M11 생성 결과
src/steno/steno_quick_dispatch.c      M0~M11 Macro 호출기
tools/generate_steno_quick.py         입력 마스크 생성기
tools/update_quick_macro_text.py      텍스트 -> Macro 키 시퀀스 생성기
```


## 6. 빌드 안정화 유지

- `cornix_right settings_reset`에는 `studio-rpc-usb-uart`를 넣지 않음
- 음절·약어·HID 출력 엔진과 퀵 Macro dispatch는 central에서만 컴파일
- 오른쪽 peripheral은 위치 이벤트와 STENO LED 수신·렌더링만 담당
- 기존 +8dBm·BLE 1M 우선·RF 출력 조절 기능 유지

## 7. 검증

```bash
python3 tools/generate_steno_dictionary.py \
  config/steno_dictionary.tsv \
  src/steno/steno_dictionary_generated.c \
  --layout config/steno_role_layout.tsv

python3 tools/generate_steno_quick.py \
  config/steno_quick_abbreviations.tsv \
  src/steno/steno_quick_generated.c

python3 tools/validate_package.py .
```

호스트 C 시험과 정적 ZMK/Zephyr API 형태 검사는 포함되어 있다. 실제 Zephyr 전체 컴파일과 실기기 동작은 GitHub Actions와 Cornix에서 최종 확인해야 한다.
