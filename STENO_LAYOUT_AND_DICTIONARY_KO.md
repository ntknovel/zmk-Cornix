# Cornix STENO 최종 배열·canonical 사전 안내 v1.7.0

## 자음 배열

```text
             Q      W      E      R      T       Y      U      I      O      P
             ㅋ     ㅈ     ㄴ     ㅎ     ㅌ      종ㅌ   종ㅎ   종ㄴ   종ㅈ   종ㅋ

             A      S      D      F      G       H      J      K      L      ;
             ㅅ     ㄱ     ㄹ     ㅇ    쌍초     쌍종   종ㅇ   종ㄹ   종ㄱ   종ㅅ

             Z      X      C      V      B       N      M      ,      .      /
             ㅊ     ㅂ     ㄷ     ㅁ     ㅍ      종ㅍ   종ㅁ   종ㄷ   종ㅂ   종ㅊ
```

## 사전 파일

- 원본: `config/steno_dictionary.tsv`
- 완성 사전 참고본: `config/abbreviation_dictionary_canonical_v0.1.json`
- 생성 결과: `src/steno/steno_dictionary_generated.c`
- 기본 활성 canonical exact-mask: 42개
- GUI 편집 퀵 Macro: 12개

재생성:

```bash
python3 tools/generate_steno_dictionary.py \
  config/steno_dictionary.tsv \
  src/steno/steno_dictionary_generated.c \
  --layout config/steno_role_layout.tsv
```

사전은 물리키가 아닌 논리 역할로 저장됩니다. ABBR_L/R은 서로 다른 역할이며,
AB2 항목만 VEXT_L 또는 VEXT_R 중 어느 쪽으로도 호출됩니다.
