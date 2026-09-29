# CORNIX STENO Studio v2.2.9 — 48번 SELECT SHIFT 복구 + 단독 RAlt 추가

기준: 바로 이전 **CORNIX STENO Studio v2.2.8**.
이번 기능 수정은 **STENO 레이어 6의 위치 48 하나**입니다. BASE 위치 48은 변경하지 않았습니다.
이 ZIP은 빌드용 전체 소스이며, 완성 UF2는 포함하지 않습니다.

## 48번의 최종 동작

| 입력 | 동작 |
|---|---|
| 48만 누르고 **500ms 미만에 떼기** | **RAlt를 한 번** 누르고 뗍니다. 속기 모드·영문 복귀 상태는 변경하지 않습니다. |
| 48을 **500ms 이상 계속 누르기** | 기존 **SELECT SHIFT**: RShift 유지 + Select Nav 레이어 7 활성화 |
| 48을 누른 채 다른 키 누르기 | 500ms를 기다리지 않고 **SELECT SHIFT**로 확정. I/J/K/L은 선택 방향키 |
| 홀드·선택 입력을 한 뒤 48 떼기 | RShift와 Select Nav 해제. **이때 RAlt를 추가 전송하지 않습니다.** |

500ms는 기존 `CONFIG_CORNIX_STENO_SOLO_CORRECTION_MS=500`과 맞춘 경계값입니다.
이 키에서 500ms는 **단독 탭과 선택 홀드를 나누는 시간**입니다. 선택 중 키를 뗄 때까지 RAlt를 무조건 출력하라는 뜻으로 구현하지 않았습니다.

```text
48 → 짧게 떼기                : RAlt 1회
48을 누른 채 I (물리 08)      : Shift+↑
48을 누른 채 J (물리 19)      : Shift+←
48을 누른 채 K (물리 20)      : Shift+↓
48을 누른 채 L (물리 21)      : Shift+→
선택 후 48 떼기              : 선택용 Shift/레이어 해제, RAlt 없음
```

첫 번째 방향키를 누르기 전에 500ms를 기다릴 필요는 없습니다. 48을 먼저 누른 뒤 방향키를 누르는 순서입니다.
단독 탭은 키를 누른 순간이 아니라 **뗀 뒤 한 번** 전송합니다.
순수 RAlt 전송에 따라 실제 OS가 한/영을 전환하는지는 기존 OS의 RAlt/한영 매핑을 따릅니다.

## 구현

기존 `select_shift` 매크로와 Select Nav의 I/J/K/L 배정은 **v2.2.8 원문 그대로** 두고, 아래 표준 ZMK hold-tap으로 감쌌습니다.

```dts
st_sel: steno_select_shift_tap {
    compatible = "zmk,behavior-hold-tap";
    #binding-cells = <2>;
    bindings = <&select_shift>, <&kp>;
    flavor = "hold-preferred";
    tapping-term-ms = <500>;
    quick-tap-ms = <0>;
    require-prior-idle-ms = <0>;
    display-name = "STENO Select Shift / RAlt solo 500ms";
};
```

STENO 위치 48의 바인딩은 `&st_sel 0 RALT`입니다. 0은 인수가 없는 기존 `select_shift` 매크로에 전달하는 자리값입니다.
재빠른 두 번째 입력이나 직전 타건 때문에 선택 기능이 강제로 탭으로 바뀌지 않도록 quick-tap / prior-idle 강제 탭은 비활성화했습니다.
`retro-tap`을 쓰지 않아 긴 선택 홀드의 해제에서 RAlt가 뒤늦게 전송되지 않습니다.

## 변경하지 않은 것

- **BASE 48:** RAlt 탭 / RCtrl 홀드·다른 키 조합, 200ms — 그대로.
- **속기 오른쪽 인코더 회전:** Shift+Page Up / Shift+Page Down — 그대로.
- v2.2.8의 BASE·NUM·MOUSE·속기 단독 기호 변경 — 그대로.
- 일반/속기 전환키와 오른쪽 인코더 클릭의 임시 영문 복귀 기능 — 그대로.
- 약어·퀵 문구·500ms 자모 판정·빠른 chord·겹받침·피로분산·교차 쌍자음 — 그대로.
- 잠금 없는 USB Studio, USB 자동 선택, BLE/RF/LED 및 하드웨어·빌드 의존성 커밋 — 그대로.

전체 400개 위치 중 STENO 48의 바인딩 하나만 바뀌었으며, 나머지 399개와 모든 인코더 배정은 같습니다.
새 동작을 붙이기 위한 hold-tap 노드, 버전/빌드 산출물 이름, 설명서·검사 파일만 함께 갱신했습니다.
구버전 안내와 검증 로그는 이력으로 보존했습니다. v2.2.8의 “Select Shift 제거” 문구는 이번 정정으로 폐기되었습니다.

## 빌드 및 Studio 저장값

기존 **GitHub ZMK 자동 빌더 v1.1.0**에서 `CORNIX_STENO_STUDIO_v2.2.9_Source.zip` 전체 소스를 선택해 빌드합니다.

```text
cornix_left_steno_studio_v2_2_9.uf2
cornix_right_steno_v2_2_9.uf2
cornix_settings_reset_v2_2_9.uf2
```

실제 빌드 성공 뒤 left/right를 각각 해당 유닛에 기록합니다. 이 ZIP의 완성 UF2 제공을 뜻하지 않습니다.
Studio에 v2.2.8 키맵을 저장했다면 새 소스 기본값보다 저장값이 우선할 수 있습니다.
필요한 개인 배정을 기록한 뒤 **Restore Stock Settings**로 이번 기본값을 불러오세요.
이 소스는 Studio 저장값이나 페어링을 자동 초기화하지 않습니다. settings_reset은 이번 정정만을 위해 먼저 넣지 않습니다.

## 검증 범위

- 400개 바인딩/인코더 비교, 단일 위치 변경 및 기존 selector 매크로·레이어 보존 검사.
- 500ms 타이밍 설정과 실제 STENO 설정값 일치 검사.
- v2.2.8의 런타임·하드웨어·사전 파일 SHA-256 보존 검사.
- 단독 499/500ms, 조기 방향키, 두 가지 해제 순서, 선택 후 RAlt 미전송, 반복 선택에 대한 **바인딩 기반 명세 모델**.
- 기존 소스/실제 속기 C 엔진/USB 호스트 회귀검사를 함께 실행.

명세 모델은 **ZMK 원본 이벤트 큐나 매크로 큐를 실행한 시험이 아닙니다.**
이 작업에서는 ARM 최종 빌드·실제 키보드·Studio 저장/재접속 시험을 수행하지 않았습니다.
검사 실행: `bash validation/run_all.sh`

## 표준 동작 참조

- https://zmk.dev/docs/keymaps/behaviors/hold-tap
- https://zmk.dev/docs/keymaps/behaviors/macros
- https://zmk.dev/docs/features/studio
