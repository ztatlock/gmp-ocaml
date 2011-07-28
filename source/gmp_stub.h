/* word size */

#define WSIZE_32_Z 12 /* 4 byte int + 4 byte int + 4 byte pointer */
#define WSIZE_64_Z 16 /* 4 byte int + 4 byte int + 8 byte pointer */

#define WSIZE_32_Q (WSIZE_32_Z * 2)
#define WSIZE_64_Q (WSIZE_64_Z * 2)

#define WSIZE_32_F 16 /* 4 byte int + 4 byte int + 4 byte int + 4 byte pointer */
#define WSIZE_64_F 24 /* 4 byte int + 4 byte int + 8 byte int + 8 byte pointer */

/* conversions */

static inline mpz_ptr Z_val(value x)
{
	return Data_custom_val(x);
}

static inline mpq_ptr Q_val(value x)
{
	return Data_custom_val(x);
}

static inline mpf_ptr F_val(value x)
{
	return Data_custom_val(x);
}

static inline __gmp_randstate_struct *Random_val(value x)
{
	return Data_custom_val(x);
}

/* hash */

#define HASH_MODULUS 4294967291UL /* compatibility for Caml-GMP */

static inline long z_hash(mpz_ptr x)
{
	return mpz_fdiv_ui(x, HASH_MODULUS);
}

static inline long q_hash(mpq_ptr x)
{
	return z_hash(mpq_numref(x)) ^ z_hash(mpq_denref(x));
}

static inline long f_hash(mpf_ptr x)
{
	long exp;
	double fra = mpf_get_d_2exp(&exp, x);
	long result = exp ^ (long)(fra * (double)LONG_MAX);
	return result;
}

/* serialize */

static inline void z_serialize(mpz_ptr x)
{
	char *image = mpz_get_str (NULL, 16, x);
	size_t length = strlen(image);
	serialize_int_4(length);
	serialize_block_1(image, length);
	free(image);
}

static inline void z_deserialize(mpz_ptr x)
{
	size_t length = deserialize_uint_4();
	char image[length + 1];
	deserialize_block_1(image, length);
	image[length] = '\0';
	int err = mpz_init_set_str(x, image, 16);
	if(err < 0) caml_failwith(__FUNCTION__);
}

static inline void q_serialize(mpq_ptr x)
{
	z_serialize(mpq_numref(x));
	z_serialize(mpq_denref(x));
}

static inline void q_deserialize(mpq_ptr x)
{
	z_deserialize(mpq_numref(x));
	z_deserialize(mpq_denref(x));
	mpq_canonicalize(x);
}

static inline void f_serialize(mpf_ptr x)
{
	mp_bitcnt_t prec = mpf_get_prec(x);
	mp_exp_t exponent;
	char *image = mpf_get_str (NULL, &exponent, 16, 0, x);
	size_t i_length = strlen(image);
	char exponent_buf[sizeof(mp_exp_t) * 2 + 1];
	size_t e_length = gmp_sprintf(exponent_buf, "%lx", (long)exponent);
	size_t length = i_length + e_length + 3;
	serialize_int_4(prec);
	serialize_int_4(length);
	if(image[0] == '-'){
		serialize_block_1("-0.", 3);
		serialize_block_1(image + 1, i_length - 1);
	}else{
		serialize_block_1("0.", 2);
		serialize_block_1(image, i_length);
	}
	serialize_block_1("@", 1);
	serialize_block_1(exponent_buf, e_length);
	free(image);
}

static inline void f_deserialize(mpf_ptr x)
{
	mp_bitcnt_t prec = deserialize_uint_4();
	size_t length = deserialize_uint_4();
	char image[length + 1];
	deserialize_block_1(image, length);
	image[length] = '\0';
	mpf_init2(x, prec);
	int err = mpf_set_str(x, image, 16);
	if(err < 0) caml_failwith(__FUNCTION__);
}

/* custom-operations */

extern struct custom_operations mlgmp_z_ops;
extern struct custom_operations mlgmp_q_ops;
extern struct custom_operations mlgmp_f_ops;