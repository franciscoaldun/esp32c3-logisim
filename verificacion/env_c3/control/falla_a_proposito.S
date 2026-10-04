# Control negativo: esta prueba DEBE fallar en el caso 3 (1 + 1 no es 3).
# Si el arnes dijera PASS aqui, sus PASS no valdrian nada.
#include "riscv_test.h"
#include "test_macros.h"

RVTEST_RV32U
RVTEST_CODE_BEGIN

  TEST_RR_OP( 2, add, 2, 1, 1 );
  TEST_RR_OP( 3, add, 3, 1, 1 );

  TEST_PASSFAIL

RVTEST_CODE_END

  .data
RVTEST_DATA_BEGIN
  TEST_DATA
RVTEST_DATA_END
