# CORNIX STENO Studio v2.2.9 — 속기 48 단독 RAlt + 기존 SELECT SHIFT

현재 기준은 KEYMAP_UPDATE_v2_2_9_KO.md / KEYMAPS_v2_2_9.html입니다.
v2.2.8에서 속기 48을 순수 RAlt 전용키로 바꾼 부분을 정정했습니다.
단독 500ms 미만 release: RAlt 1회. 500ms 홀드 또는 다른 키와 조합: 기존 SELECT SHIFT.
BASE 48의 200ms RAlt/RCtrl 및 v2.2.8의 다른 요청 변경은 유지합니다.
아래의 v2.2.8 이하 안내·로그·배열표는 과거 기록이며, 현재 48의 지시보다 우선하지 않습니다.

---

# CORNIX STENO Studio v2.2.8

현재 변경 키맵과 빌드 안내는 KEYMAP_UPDATE_v2_2_8_KO.md를 우선합니다.
아래 v2.2.7 이하 내용은 이전 기능/검증 참고 기록입니다. 이전 배열표의 단독키 출력과 00+12 콤보는 현재와 다릅니다.

---

# CORNIX STENO v2.2.7 — USB ZMK Studio / 상시 잠금 해제

## 이 ZIP의 정체

CORNIX STENO **v2.2.6 MIRRORED_ONSET 전체본**에 USB Studio 지원을 더한 **빌드용 전체 소스**입니다.
HalmaxCornix 비속기 키맵이나 KLOR 키맵으로 교체하지 않았습니다.
완성 UF2는 들어 있지 않습니다. 소스·호스트 시험은 실시했지만, 이 환경에서 ARM 최종 컴파일 및 실물 Studio 접속은 실행하지 않았습니다.
이 문서가 이번 릴리스의 사용 기준이며, 기존 v2.2.6 이하 문서/로그는 입력 규칙과 과거 검증 참고용으로 보존했습니다.

## 바로 빌드하기

1. 기존 **GitHub ZMK 자동 빌더 v1.1.0**의 `START.cmd`를 실행합니다.
2. 파일 선택에서 `CORNIX_STENO_STUDIO_v2.2.7_Source.zip`을 선택합니다. 소스 ZIP은 풀 필요가 없습니다.
3. 기존 저장소 URL을 입력하거나 **새 저장소 만들기**를 체크합니다. `HalmaxCornix`라는 이름의 레포는 필요 없습니다.
4. GitHub 토큰을 입력하고 빌드 버튼을 누릅니다.

소스 전체를 선택해야 합니다. 비속기 소스나 이전 UF2 ZIP과 합치지 마세요.
자동 빌더 v1.1.0의 실제 소스 검사 함수를 이용해 이 ZIP의 세 타깃/워크플로를 인식하는지 로컬에서 검사했습니다.
온라인 토큰 인증·게시·빌드·다운로드는 별도이며, 로컬 호스트 검사 통과가 온라인 빌드 성공을 의미하지 않습니다.

## 생성 대상

| artifact-name / UF2 이름 | 용도 |
|---|---|
| `cornix_left_steno_studio_v2_2_7.uf2` | 왼쪽 central, 한국어 속기 + USB Studio |
| `cornix_right_steno_v2_2_7.uf2` | 오른쪽 peripheral, 원래 무선 split + STENO LED |
| `cornix_settings_reset_v2_2_7.uf2` | 초기화 전용, STENO/Studio 서버 없음 |

양쪽 일반 펌웨어와 reset 타깃의 Actions 성공을 확인합니다. 평상시 업데이트에는 left/right만 각각 해당 유닛에 넣습니다.
이번 Studio 추가만을 이유로 settings reset을 넣지 마세요. 원래 정상 펌웨어는 복구용으로 보관하세요.
이것은 CORNIX nRF52840용이며 RP2040 KLOR용이 아닙니다.

## 연결 동작

- USB Studio는 **왼쪽 유닛**에서만 제공합니다.
- `CONFIG_ZMK_STUDIO_LOCKING=n`: 별도 unlock 키 입력이 없습니다.
- `CONFIG_ZMK_STUDIO_TRANSPORT_BLE=n`: Studio 설정 연결만 USB 전용입니다. 일반 BLE 타이핑/좌우 BLE split은 유지합니다.
- 데이터 USB가 HID 사용 가능 상태가 되면 비차단 작업으로 USB 출력을 한 번 선택합니다.
- 충전 전용 연결은 출력 선택을 강제하지 않습니다. 연결 뒤 사용자가 수동으로 OUT_BLE를 고른 경우에는 다음 USB 재연결까지 존중합니다.
- 재연결 시 다시 USB 출력을 선택합니다. 잠금 기능 자체는 계속 비활성입니다.

왼쪽에 **데이터 USB 케이블** 연결 → Chrome/Edge의 ZMK Studio 또는 네이티브 앱 → 장치 선택/접근 허용 → 키맵 편집/저장 순서입니다.
펌웨어의 잠금은 없지만 브라우저 장치 권한 승인은 남습니다. USB를 꽂으면 웹사이트가 저절로 열리는 기능은 아닙니다.
잠금 없는 설정이므로 신뢰하는 PC에 연결해서 사용하세요.

## 보존한 속기 기능

- 기존 8개 레이어, 각 50위치, 13개 콤보, 좌우 인코더 배정
- ㅂㅈㄷㄱㅅ 미러 배열과 초성/종성/엄지 역할
- canonical 약어 43개 + Quick Macro 12개
- 단독 수정/직접 자모 500ms, modifier dual-role 200ms
- 전역 key-up 역할 삭제 OFF(0ms), 마지막 release에서 일반 조합 확정
- 합법 겹받침 rolling 90ms의 국소 판정
- 종성 자음을 초성으로 사용하는 ㅗ/ㅡ/ㅜ 피로분산 규칙, 전용 ,/. 교차 쌍자음
- `뿌`, `빠`, 기존 `박` 보존과 약어/편집/방향키 처리
- 약어L + 전용쌍초 / 약어R + 전용쌍종 이동
- 기호L + 전용쌍초 / 기호R + 전용쌍종 선택 이동
- STENO LED, RF 출력, BLE, 보드/ZMK/표시 모듈 고정 커밋

`config/cornix.keymap`, `config/cornix_steno.dtsi`, 사전/역할 TSV, 모든 `src/steno/` 파일은 v2.2.6과 바이트 단위로 같습니다.
두 behavior 파일(arrow, 내부 LED)은 **Studio 파라미터 메타데이터만** 바로잡았습니다. 기존 key-down/key-up 입력 처리 함수는 바꾸지 않았습니다.
파라미터가 있는 동작을 Studio에 '파라미터 없음'으로 알려주지 않도록 하는 변경입니다.

## Studio 편집 범위와 속기 고정 영역

일반 레이어의 기본 키/동작 배정을 편집·저장하는 용도부터 사용하세요. 기존의 속기 역할 behavior도 표시될 수 있지만, 이것이 임의 재배열까지 지원한다는 뜻은 아닙니다.

**STENO 레이어 6과 SELECT_NAV 레이어 7의 번호/순서, 속기 자모·엄지·기호·약어 역할 위치는 유지하세요.**
기존 엔진에는 위치에 의존하는 방향키/숫자/피로분산·보호 규칙이 있으며, 이번 작업은 그 알고리즘을 동적 재매핑용으로 바꾸지 않았습니다.
Studio에서 속기 키를 일반 `&kp`로 바꾸면 해당 키가 속기 조합에 참여하지 않습니다. 내부 `st_led` 동작은 일반 키에 배정하지 마세요.

약어 출력 문자열, Quick Macro 내부 문자열, 콤보 정의, 인코더 회전 바인딩, 탭/홀드 상세시간, 500ms 속기 판정값은 이번 Studio UI에서 직접 편집하는 항목이 아닙니다. 바꾸려면 소스 수정 후 재빌드합니다.
예비 레이어는 요청 없이 추가하지 않았습니다. 기존 8개 레이어를 보존했습니다.

Studio에 저장된 키맵은 이후 새 `.keymap` 기본값보다 우선할 수 있습니다. 예전에 같은 장치에 비속기 Studio 키맵을 저장했다면 앱의 **Restore Stock Settings**로 이번 펌웨어의 기본 키맵을 불러와야 할 수 있습니다.
이 작업은 저장된 Studio 키맵 변경을 버리는 동작이므로 필요한 설정을 먼저 기록하세요. 이번 소스는 저장 설정이나 페어링을 자동 초기화하지 않습니다.

## 개발자 검사

기존 역할 12,383건, canonical 입력순서 172건, 복합모음 38건, 쌍자음 60건, 피로분산 189건과 단독 입력 경계 검사를 유지합니다. USB 선택·메타데이터·빌드 설정 검사도 별도로 실행합니다.

```bash
bash validation/run_all.sh
```

실제 ARM 빌드 폴더가 있을 때만 해당 설정을 검사합니다.

```bash
python3 tools/check_built_studio.py build/left/zephyr/.config left
python3 tools/check_built_studio.py build/right/zephyr/.config right
python3 tools/check_built_studio.py build/reset/zephyr/.config reset
```

실제 빌드에는 compile-time 가드도 들어 있습니다. Studio locking/USB transport/central/STENO 설정이나 50위치 물리 레이아웃이 맞지 않으면 조용히 잘못된 펌웨어를 만드는 대신 빌드를 중단합니다.
최종 실기 확인: 좌우 타이핑 → 한국어 속기/뿌/빠/약어 → USB Studio 무잠금 연결 → 일반 키 한 개 저장·재연결 → Bluetooth 재연결.

## 참조

- ZMK Studio 사용/제약/USB snippet: https://zmk.dev/docs/features/studio
- locking 옵션: https://zmk.dev/docs/config/studio
- behavior metadata 구조: https://github.com/zmkfirmware/zmk/blob/main/app/include/drivers/behavior.h
- Studio 패치의 USB 자동 선택 모듈은 앞서 제공한 HalmaxCornix Studio v1.3.1 모듈을 속기판에 독립 이식한 것입니다. 비속기 키맵은 가져오지 않았습니다.

검증 한계: 고정 커밋 의존성은 변경하지 않았으나, 이 실행 환경에서는 GitHub 원본을 내려받는 DNS 접속이 실패했고 ARM 툴체인이 확인되지 않았습니다. 실기나 최종 ARM 빌드 성공을 주장하지 않습니다.
