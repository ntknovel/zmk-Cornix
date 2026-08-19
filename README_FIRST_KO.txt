Cornix STENO v2.2.2 — 빠른 적용
==================================

1. 정상 저장소에서 새 브랜치를 만듭니다.

   cd /workspaces/zmk-Cornix
   git status
   git switch -c v2.2.2

2. v2.2.1 기준 OVERWRITE PATCH ZIP을 저장소 루트에 올리고 덮어씁니다.

   unzip -o Cornix_ZMK_STENO_v2.2.1_TO_v2.2.2_FULL_AUDIT_OVERWRITE_PATCH.zip -d .
   rm Cornix_ZMK_STENO_v2.2.1_TO_v2.2.2_FULL_AUDIT_OVERWRITE_PATCH.zip

3. 검증합니다.

   python3 tools/validate_package.py .
   bash validation/run_host_tests.sh

4. 둘 다 PASS한 뒤 커밋·푸시합니다.

   git add -A
   git commit -m "Update Cornix STENO to v2.2.2 audited input engine"
   git push -u origin v2.2.2

5. GitHub Actions의 left/right/settings_reset 결과를 확인합니다.

상세 입력 규칙: FINAL_INPUT_REFERENCE_KO.md
감사 수정 내역: V2.2.2_FULL_AUDIT_FIX_KO.txt
