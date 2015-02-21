#define ZEPHIR_CONCAT_SS(result, op1, op2) \
	 zephir_concat_ss(&result, op1, sizeof(op1)-1, op2, sizeof(op2)-1, 0);
#define ZEPHIR_SCONCAT_SS(result, op1, op2) \
	 zephir_concat_ss(&result, op1, sizeof(op1)-1, op2, sizeof(op2)-1, 1);

#define ZEPHIR_CONCAT_SV(result, op1, op2) \
	 zephir_concat_sv(&result, op1, sizeof(op1)-1, op2, 0);
#define ZEPHIR_SCONCAT_SV(result, op1, op2) \
	 zephir_concat_sv(&result, op1, sizeof(op1)-1, op2, 1);

#define ZEPHIR_CONCAT_SVS(result, op1, op2, op3) \
	 zephir_concat_svs(&result, op1, sizeof(op1)-1, op2, op3, sizeof(op3)-1, 0);
#define ZEPHIR_SCONCAT_SVS(result, op1, op2, op3) \
	 zephir_concat_svs(&result, op1, sizeof(op1)-1, op2, op3, sizeof(op3)-1, 1);

#define ZEPHIR_CONCAT_VS(result, op1, op2) \
	 zephir_concat_vs(&result, op1, op2, sizeof(op2)-1, 0);
#define ZEPHIR_SCONCAT_VS(result, op1, op2) \
	 zephir_concat_vs(&result, op1, op2, sizeof(op2)-1, 1);

#define ZEPHIR_CONCAT_VV(result, op1, op2) \
	 zephir_concat_vv(&result, op1, op2, 0);
#define ZEPHIR_SCONCAT_VV(result, op1, op2) \
	 zephir_concat_vv(&result, op1, op2, 1);

#define ZEPHIR_CONCAT_VVV(result, op1, op2, op3) \
	 zephir_concat_vvv(&result, op1, op2, op3, 0);
#define ZEPHIR_SCONCAT_VVV(result, op1, op2, op3) \
	 zephir_concat_vvv(&result, op1, op2, op3, 1);


void zephir_concat_ss(zval *result, const char *op1, uint32_t op1_len, const char *op2, uint32_t op2_len, int self_var);
void zephir_concat_sv(zval *result, const char *op1, uint32_t op1_len, zval *op2, int self_var);
void zephir_concat_svs(zval *result, const char *op1, uint32_t op1_len, zval *op2, const char *op3, uint32_t op3_len, int self_var);
void zephir_concat_vs(zval *result, zval *op1, const char *op2, uint32_t op2_len, int self_var);
void zephir_concat_vv(zval *result, zval *op1, zval *op2, int self_var);
void zephir_concat_vvv(zval *result, zval *op1, zval *op2, zval *op3, int self_var);
void zephir_concat_function(zval *result, zval *op1, zval *op2);