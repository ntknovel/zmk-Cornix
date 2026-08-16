# Cornix BASE 상태 LED + STENO 입력 LED v1.3

## 실제 상태 LED 수

공개 Cornix 보드 정의와 제품 매뉴얼은 각 반쪽의 상태 RGB LED를 2개로 정의한다. 따라서 현재 펌웨어가 개별 제어하는 물리 LED는 좌우 2개씩 총 4개다.

## BASE 동작

BASE에서는 공식 `cornix_indicator`와 `zmk-rgbled-widget`가 그대로 동작한다.

- Bluetooth 프로필·연결·광고·분리 표시
- 충전 및 배터리 부족 표시
- 연결 완료 등의 임시 표시가 끝나면 소등
- 모든 표시가 꺼진 뒤 약 1초가 지나면 WS2812 전원 레일 차단

STENO용 On/Off 설정은 BASE 상태 표시를 끄지 않는다.

## STENO 동작

```text
모드 진입 직후       네 LED 모두 600ms 켜짐
진입 확인 후 대기    네 LED 모두 꺼짐
홀드 1개             1번 LED 켜짐
홀드 2개             1~2번 LED 켜짐
홀드 3개             1~3번 LED 켜짐
홀드 4개 이상        네 LED 모두 켜짐
전체 release         네 LED 모두 꺼짐
BASE 복귀            공식 상태 표시기로 제어권 반환
```

전역 순서는 다음과 같다.

```text
1번·2번 = central 반쪽의 두 LED
3번·4번 = peripheral 반쪽의 두 LED
```

이 순서는 어느 손에서 키를 눌렀는지 표시하는 것이 아니라 **현재 인식된 물리 STENO 입력키의 총 홀드 수**를 보여 준다.

## 카운트 대상

포함:

- 초성·종성·엄지 모음
- VEXT
- 약어·기호 marker
- `ㅜ/Enter`, `ㅏ/Space`, `EDIT/Backspace`
- `LCtrl/[`, `LAlt/]`
- Tab/Copy

제외:

- BASE/STENO 모드 토글
- RAlt 한영 전환
- 인코더 회전
- 일반 RCtrl

## On/Off

Fn-BT 레이어의 Base G 물리 위치:

```text
STENO LED On/Off
```

- OFF: STENO 진입 flash와 홀드 카운터 모두 끔
- ON: STENO 중에 켜면 네 LED가 600ms 켜져 재활성화를 알린 뒤 현재 홀드 수 표시
- BASE의 Bluetooth·배터리 표시에는 영향 없음
- 플래시 저장하지 않는 런타임 설정이므로 재부팅하면 ON으로 시작

## 전력

STENO 대기 상태는 LED를 끈 상태이므로, 이전처럼 모드 내내 흰색을 유지하는 설계보다 대기 소비를 줄인다. 모든 LED가 검정 상태가 되면 공식 위젯의 전원 관리가 WS2812 전원 레일을 자동으로 끈다.
