# Cornix STENO v1.6.2 — settings_reset 빌드 수정

정상 좌/우 펌웨어는 이미 빌드에 성공했고, 선택 사항인 settings_reset 행만 실패한 상태를 수정한다.

변경 사항:

- `build.yaml`의 settings_reset 행에서 `snippet: nrf52840-nosd` 제거
- Cornix 공식 예시와 같은 `board + settings_reset shield` 구성 사용
- 공유 `config/cornix.conf`에서 STENO 설정 제거
- 정상 좌/우 빌드에만 포함되는 `cornix_steno_led.conf`에서 STENO 코어 활성화
- `CONFIG_CORNIX_STENO` 기본값을 `n`으로 변경
- `config/settings_reset.conf`에서 STENO와 pointing을 명시적으로 비활성화

이로써 reset 이미지에는 키맵·약어·LED 오버레이 엔진이 들어가지 않고, 페어링 설정 삭제에 필요한 최소 구성만 남는다.
