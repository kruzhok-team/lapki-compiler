
//
// Потоковый приёмопередатчик данных посредством ИК канала
// Прямая адаптация акустического модема
//
// TODO: упростить для ИК диапазона
//

// ============================================================================
// Передатчик
// ============================================================================

//
// На blg-mb-1-a12 инфракрасный излучатель управляется через PE12,
// на PE12 доступен выход таймера TIM1_CH3N
//
// Мы задействуем его для генерации несущей + модуляции
//
extern "C" {
	static int ir_emitter_period = 0;

	void ir_modem_init_emitter(void) {
		RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
		initPin_PP(GPIOE, 12);
	}

	static void ir_emitter_radiate_6000() {
		ir_emitter_period = 48000.0/6000.0 + 0.5;
	}

	static void ir_emitter_radiate_2730() {
		ir_emitter_period = 48000.0/2730.0 + 0.5;
	}

	static void ir_emitter_kill() {
		ir_emitter_period = 0;
	}

	typedef void (*tx_func_t)();

	static void emitter_idle();
	static void preamble();
	static void training();
	static void bits();

	static struct {
		tx_func_t fn;
		int tick;

		int preamble_ttl;
		int bit_holdoff;
		int bit_pend;
		int bit_ttl;

		int not_empty;
		int bit_queue;
	} tx = { .fn = emitter_idle };

	static void preamble() {
		if (tx.preamble_ttl > 0) {
			tx.preamble_ttl--;
			return;
		}

		ir_emitter_kill();

		// 135 = расстояние от преамбулы до бита
		// 64 = длительность заполнения бита
		tx.bit_holdoff = 135 - 64;
		tx.bit_ttl = 64;
		tx.fn = training;
	}

	// Бит для оценки состояния канала
	// Помогает приемнику определить уровень логической единицы
	static void training() {
		if (tx.bit_holdoff > 0) {
			tx.bit_holdoff--;

			if (tx.bit_holdoff == 0)
				ir_emitter_radiate_2730();

			return;
		}

		if (tx.bit_ttl > 0) {
			tx.bit_ttl--;
			return;
		}

		ir_emitter_kill();

		// 595 = расстояние между битами
		// 64 = длительность заполнения бита
		tx.bit_holdoff = 595 - 64;
		tx.bit_ttl = 64;
		tx.bit_pend = 8;
		tx.fn = bits;
	}

	static void bits() {
		if (tx.bit_holdoff > 0) {
			tx.bit_holdoff--;

			if (tx.bit_holdoff == 0) {
				if (tx.bit_queue & 0x80)
					ir_emitter_radiate_2730();

				tx.bit_queue <<= 1;
			}

			return;
		}

		if (tx.bit_ttl > 0) {
			tx.bit_ttl--;
			return;
		}

		ir_emitter_kill();

		if (tx.bit_pend > 0) {
			tx.bit_holdoff = 595 - 64;
			tx.bit_ttl = 64;
			tx.bit_pend--;
			return;
		}

		tx.not_empty = 0;
		tx.fn = emitter_idle;
	}

	static void emitter_idle() {
		if (tx.not_empty == 0)
			return;

		ir_emitter_radiate_6000();

		// 64 = длительность преамбулы
		tx.preamble_ttl = 64;
		tx.fn = preamble;
	}

	// Диспетчиризирующая функция
	static void ir_tx_advance() {
		if (tx.fn) {
			tx.fn();
		}
		tx.tick++;

		setPin_PP(GPIOE, 12, (tx.tick % ir_emitter_period) < (ir_emitter_period/2) ? ON : OFF);
	}

	// ============================================================================
	// Приемник
	// ============================================================================

	// Полосовой фильтр, IIR, ~2.73 КГц
	static float filter_2730_next(float x) {
		static float y_past[3] = {0.0f};
		static float x_past[3] = {0.0f};

		const float feedforw[] = {1.0/40.0, 1.0/80.0, -1.0/27.0};
		const float feedback[] = {1.0, -9.0/5.0, 11.0/12.0};

		x_past[2] = x_past[1];
		x_past[1] = x_past[0];
		x_past[0] = x;

		y_past[2] = y_past[1];
		y_past[1] = y_past[0];
		y_past[0] = 	feedforw[0] * x_past[0] +
				feedforw[1] * x_past[1] +
				feedforw[2] * x_past[2] -
				feedback[1] * y_past[1] -
				feedback[2] * y_past[2];

		return y_past[0];
	}

	// Полосовой фильтр, IIR, 6.0 КГц
	static float filter_6000_next(float x) {
		static float y_past[3] = {0.0f};
		static float x_past[3] = {0.0f};

		const float feedforw[] = {1.0/33.0, 1.0/412.0, -1.0/31.0};
		const float feedback[] = {1.0, -11.0/8.0, 14.0/15.0};

		x_past[2] = x_past[1];
		x_past[1] = x_past[0];
		x_past[0] = x;

		y_past[2] = y_past[1];
		y_past[1] = y_past[0];
		y_past[0] = 	feedforw[0] * x_past[0] +
				feedforw[1] * x_past[1] +
				feedforw[2] * x_past[2] -
				feedback[1] * y_past[1] -
				feedback[2] * y_past[2];

		return y_past[0];
	}


	#define ABS(val) ((val) > 0 ? (val) : -(val))

	typedef void (*ir_rx_func_t)(float u, float v);

	static void ir_preamble_detect(float u, float v);
	static void ir_preamble_sync(float u, float v);
	static void ir_link_training(float u, float v);
	static void ir_bit_sampling(float u, float v);

	static struct {
		ir_rx_func_t fn;
		uint32_t tick;

		int preamble_ttl;
		int preamble_loc;
		float preamble_max;

		int training_ttl;
		float training_max;

		float bit_thresh;
		int bit_holdoff;
		int bit_pend;
		int bit_ttl;
		int bit_acc;
		float bit_max;

		int have_byte;
		int is_enabled;
	} ir_rx = { .fn = ir_preamble_detect, 0 };

	static void ir_preamble_detect(float u, float v) {
		if (ABS(v) > 50) { // Порог детектирования сигнала
			ir_rx.preamble_ttl = 100;
			ir_rx.preamble_loc = 100;
			ir_rx.preamble_max = ABS(v);
			ir_rx.fn = ir_preamble_sync;
		}
	}

	static void ir_preamble_sync(float u, float v) {
		if (ir_rx.preamble_ttl >= 0) {
			ir_rx.preamble_ttl--;

			if (ABS(v) > ir_rx.preamble_max) {
				ir_rx.preamble_max = ABS(v);
				ir_rx.preamble_loc = 100 - ir_rx.preamble_ttl;
			}

			return;
		}

		if (ir_rx.preamble_loc >= 0) {
			ir_rx.preamble_loc--;
			return;
		}

		ir_rx.training_ttl = 100;
		ir_rx.training_max = 0.0f;
		ir_rx.fn = ir_link_training;
	}

	static void ir_link_training(float u, float v) {
		if (ir_rx.training_ttl >= 0) {
			ir_rx.training_ttl--;

			if (ABS(u) > ir_rx.training_max) {
				ir_rx.training_max = ABS(u);
			}

			return;
		}

		ir_rx.bit_thresh = 0.75f * ir_rx.training_max;
		ir_rx.bit_holdoff = 495;
		ir_rx.bit_pend = 8;
		ir_rx.bit_ttl = 100;
		ir_rx.bit_max = 0.0f;
		ir_rx.bit_acc = 0;
		ir_rx.fn = ir_bit_sampling;
	}

	static void ir_bit_sampling(float u, float v) {
		if (ir_rx.bit_holdoff >= 0) {
			ir_rx.bit_holdoff--;
			return;
		}

		if (ir_rx.bit_ttl >= 0) {
			ir_rx.bit_ttl--;

			if (ABS(u) > ir_rx.bit_max) {
				ir_rx.bit_max = ABS(u);
			}

			return;
		}

		int bit_detected = (ir_rx.bit_max >= ir_rx.bit_thresh);

		ir_rx.bit_pend--;
		ir_rx.bit_acc += (bit_detected << ir_rx.bit_pend);

		if (ir_rx.bit_pend > 0) {
			ir_rx.bit_holdoff = 495;
			ir_rx.bit_ttl = 100;
			ir_rx.bit_max = 0.0f;
			return;
		}

		ir_rx.fn = ir_preamble_detect;
		ir_rx.have_byte = 1;
	}

	int ir_rx_is_available() {
		return ir_rx.have_byte;
	}

	int ir_rx_read_byte() {
		ir_rx.have_byte = 0;
		return ir_rx.bit_acc;
	}

	// Диспетчеризирующая функция
	// u     2.9 кГц сигнал
	// v     6.0 кГц сигнал
	static void ir_rx_advance(float u, float v) {
		if (ir_rx.fn) {
			ir_rx.fn(u, v);
		}
		ir_rx.tick++;
	}


	void ir_modem_init() {
		ir_rx.is_enabled = 1;
		ir_modem_init_emitter();
	}
}
