# Cornix STENO canonical exact-mask 약어 42개

완성된 `약어사전파일_v0.1`에서 `enabled_by_default=true`인 42개만 컴파일 기본값으로 사용합니다.

| 분류 | 출력 | 논리 역할 | 선택자 |
|---|---|---|---|
| 기본 | 점점 | `I_J + F_J` | `-` |
| 기본 | 진짜 | `I_J + F_J + F_DOUBLE` | `-` |
| 기본 | 결국 | `I_G + F_G` | `-` |
| 기본 | 그렇게 | `I_G + I_R + F_G` | `-` |
| 기본 | 그렇군 | `I_G + F_R + F_G` | `-` |
| 기본 | 그런 | `I_G + F_R` | `-` |
| 기본 | 그럼 | `I_G + F_R` | `ABBR_L` |
| 기본 | 그래도 | `I_G + I_R + F_D` | `-` |
| 기본 | 그런데 | `I_G + I_R + F_D` | `ABBR_L` |
| 기본 | 그럼에도 | `I_G + I_R + I_NG + F_D` | `-` |
| 기본 | 그대로 | `I_G + I_D + F_R` | `-` |
| 기본 | 갑자기 | `I_G + F_J + I_DOUBLE` | `-` |
| 기본 | 완전히 | `I_NG + I_J + F_H` | `-` |
| 기본 | 조용히 | `I_NG + F_J + F_H` | `-` |
| 기본 | 되었 | `I_D + I_NG` | `-` |
| 기본 | 근데 | `I_G + F_D` | `-` |
| 기본 | 그때 | `I_G + F_D + F_DOUBLE` | `-` |
| 접속 | 게다가 | `I_G + I_D + I_DOUBLE` | `-` |
| 접속 | 그리고 | `I_G + I_R + I_DOUBLE` | `-` |
| 접속 | 그러자 | `I_G + I_R + F_J` | `-` |
| 접속 | 그러기 | `I_G + I_R + I_DOUBLE` | `ABBR_L` |
| 접속 | 그렇지 | `I_G + I_R + F_J` | `ABBR_L` |
| 접속 | 하지만 | `I_H + I_J + F_M` | `-` |
| 어미 | 였다 | `F_NG + F_D` | `-` |
| 어미 | 없다 | `F_NG + F_D` | `ABBR_R` |
| 어미 | 었다 | `F_NG + F_D` | `ABBR_L` |
| 어미 | 았다 | `F_NG + F_D` | `ABBR_L + ABBR_R` |
| 어미 | 이었다 | `F_NG + F_D` | `VEXT_L` |
| 어미 | 었었다 | `F_NG + F_D` | `VEXT_L + ABBR_R` |
| 어미 | 있다 | `F_NG + F_D` | `VEXT_L + ABBR_L` |
| 어미 | 에다 | `I_NG + F_D` | `ABBR_R` |
| 어미 | 했다 | `F_H + F_D` | `-` |
| 어미 | 됐다 | `F_DOUBLE + F_D` | `-` |
| 어미 | 는데 | `F_N` | `VEXT_L` |
| 어미 | 을지도 | `F_NG + F_J + F_D` | `-` |
| 어미 | 어졌다 | `F_NG + F_J + F_D` | `ABBR_R` |
| 어미 | 지만 | `F_J + F_M` | `-` |
| 어미 | 습니다 | `F_S + F_D` | `VEXT_L` |
| 어미 | 었으면 | `F_M` | `VEXT_L + ABBR_R` |
| 어미 | 하는게 | `F_H + F_N + F_G` | `-` |
| 어미 | 오는게 | `F_NG + F_N + F_G` | `-` |
| 어미 | 가는게 | `F_DOUBLE + F_N + F_G` | `-` |

## 핵심 판정

- 일반 자음 골격 약어는 약어키 없이 exact mask로 직접 조회합니다.
- `ABBR_L`과 `ABBR_R`은 서로 다른 선택자입니다.
- `AB2`만 좌우 VEXT 중 어느 쪽으로도 호출할 수 있습니다.
- 12개 퀵 약어는 기존 `quick_0`~`quick_11` ZMK Macro를 그대로 사용합니다.
- 미등록 exact mask는 추측하지 않고 무출력합니다.
