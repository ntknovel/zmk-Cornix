# Cornix BASE 상태 LED + STENO 입력·약어 LED v1.8.0

## 실제 상태 LED 수

Cornix는 각 반쪽 2개, 총 4개의 상태 RGB LED를 사용한다.

```text
논리 0 = 왼쪽 안쪽
논리 1 = 왼쪽 바깥쪽
논리 2 = 오른쪽 바깥쪽
논리 3 = 오른쪽 안쪽
```

## BASE 동작

BASE에서는 공식 `cornix_indicator`와 `zmk-rgbled-widget`가 담당한다.

- Bluetooth 프로필·연결·광고·분리 표시
- 충전 및 배터리 부족 표시
- 임시 표시가 끝나면 소등
- 모든 표시가 꺼진 뒤 위젯의 전원 관리가 WS2812 전원 레일 차단

STENO LED On/Off는 BASE 상태 표시를 끄지 않는다.

## STENO 일반 입력

```text
모드 진입 직후       네 LED 모두 600ms 흰색
진입 확인 후 대기    모두 OFF
홀드 1개             1번 흰색
홀드 2개             1~2번 흰색
홀드 3개             1~3번 흰색
홀드 4개 이상        네 LED 모두 흰색
전체 release         모두 OFF
BASE 복귀            공식 상태 표시로 제어권 반환
```

홀드 수에는 자음, 모음, 겹모음, 약어·기호 marker, dual key, Tab/Copy가 포함된다.
모드 토글, 언어 전환, 인코더 회전은 제외한다.

## 약어 카테고리

현재 exact mask가 등록 약어로 확인되거나 약어 출력이 확정되면 카테고리 색을 표시한다.
출력 확정 후 350ms 동안 유지한다.

| 분류 | LED |
|---|---|
| 기본 약어·Quick Macro | 왼쪽 두 LED 초록 |
| 접속 약어 | 가운데 두 LED 노랑 |
| 어미 약어 | 오른쪽 두 LED 파랑 |

카테고리 후보가 아닌 일반 STENO 입력으로 돌아오면 다시 흰색 홀드 카운터를 사용한다.

## On/Off

Fn-BT 레이어의 `STENO LED On/Off`:

- OFF: 진입 flash, 홀드 카운터, 약어 카테고리 표시를 모두 끔
- ON: 진입 확인 flash 후 현재 상태 표시
- BASE Bluetooth·배터리 표시에는 영향 없음
- 런타임 설정이므로 재부팅하면 기본 ON
