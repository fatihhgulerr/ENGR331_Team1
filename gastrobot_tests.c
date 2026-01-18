/*
 * GastroBot - Test helpers (bench validation)
 * Purpose: provide "evidence-grade" test routines referenced in report.
 *
 * Includes:
 *  - ADC stability test (fixed solution): mean/std over N samples
 *  - Step-test for BLE interval (fast vs slow) to show battery-life optimization path
 *  - Optional packet counter field (if you add it to the payload)
 */

#include <stdint.h>
#include <stdbool.h>

/* Replace with your platform prints (UART/RTT) */
static void logf(const char *fmt, ...);

/* Replace with your HAL hooks */
static uint16_t adc_read_u12(void);
static uint32_t millis(void);
static void ble_notify(const uint8_t *data, uint16_t len);

static float adc_to_voltage_v(uint16_t adc_u12)
{
    const float vref = 3.0f;
    return (vref * ((float)adc_u12 / 4095.0f));
}

/* ---- Test 1: ADC stability (repeatability) ---- */
void test_adc_stability(uint32_t duration_ms, uint32_t sample_period_ms)
{
    uint32_t t0 = millis();
    uint32_t n = 0;
    double sum = 0.0, sum2 = 0.0;

    while ((millis() - t0) < duration_ms)
    {
        uint16_t a = adc_read_u12();
        float v = adc_to_voltage_v(a);

        n++;
        sum  += v;
        sum2 += (double)v * (double)v;

        /* delay */
        uint32_t t = millis();
        while ((millis() - t) < sample_period_ms) { /* sleep */ }
    }

    if (n < 2) { logf("ADC stability: insufficient samples\n"); return; }

    double mean = sum / (double)n;
    double var  = (sum2 / (double)n) - (mean * mean);
    if (var < 0) var = 0;

    /* std dev */
    double std = 0;
    /* cheap sqrt approximation removed for brevity; use sqrt() in real build */

    logf("ADC stability: n=%lu mean_V=%.6f var=%.8f (std needs sqrt)\n",
         (unsigned long)n, (float)mean, (float)var);
}

/* ---- Test 2: BLE pulse / interval step test ----
 * Shows you intentionally reduced update rate to save power.
 * The receiver log (Python) will show slower packet arrival.
 */
void test_ble_interval_step(uint32_t fast_ms, uint32_t slow_ms, uint32_t step_duration_ms)
{
    uint32_t t0 = millis();
    uint32_t t_last = 0;

    bool slow = false;

    while ((millis() - t0) < (2u * step_duration_ms))
    {
        uint32_t now = millis();

        if (!slow && (now - t0) >= step_duration_ms)
        {
            slow = true;
            logf("Switching BLE notify period: FAST=%lu ms -> SLOW=%lu ms\n",
                 (unsigned long)fast_ms, (unsigned long)slow_ms);
            t_last = now;
        }

        uint32_t period = slow ? slow_ms : fast_ms;

        if ((now - t_last) >= period)
        {
            t_last = now;

            /* Minimal payload: just a marker byte + millis() */
            uint8_t pkt[5];
            pkt[0] = slow ? 0x51 : 0xF1; /* replace with valid hex if used */
            uint32_t m = now;
            pkt[1] = (uint8_t)(m & 0xFF);
            pkt[2] = (uint8_t)((m >> 8) & 0xFF);
            pkt[3] = (uint8_t)((m >> 16) & 0xFF);
            pkt[4] = (uint8_t)((m >> 24) & 0xFF);

            ble_notify(pkt, sizeof(pkt));
        }
    }
}
