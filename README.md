# Lab work 6: LED blinking, UART transmission etc.

Authors (team):

Daryna Shevchuk: https://github.com/dasha-pn

Oleksandr Lykhanskyi: https://github.com/LMTMARS

## Prerequisites

- OS - Windows
- PSoC Creator 4.4
- PSoC CY8CKIT-042-BLE-A

## Division of work:

Daryna Shevchuk:
- Task 1 + additional for task 1
- Task 3 Part 1 + additional for task 3

Oleksandr Lykhanskyi:
- Task 2 + additional for task 2
- Task 3 Part 2

## Additional tasks:
- Task 1: adjust the LED brightness using PWM.
- Task 2: extend the number output function to support fixed-point numbers.
You need to add one more parameter — the position of the decimal point on the display.
- Task 3: add support for displaying floating-point numbers

## Task 1.1 – Simple LED blinking

### What was done

Three Digital Output Pins were created and connected to onboard LEDs:

- `Pin_LED` → **P2[6]** (Red)  
- `Pin_Green` → **P3[6]** (Green)  
- `Pin_Blue` → **P3[7]** (Blue)

> In the starter files Blue was mapped to **P3[0]**, which was corrected to **P3[7]**.

Each LED is connected through a **560 Ω** resistor.

### Program logic

Inside an infinite loop:

1. Turn **Red + Blue ON**, Green OFF  
2. Wait **300 ms**
3. Turn **Red + Green ON**, Blue OFF  
4. Wait **300 ms**
5. Repeat

This produces alternating blinking with a 300 ms period.

### Result

LEDs blink in different color combinations with the chosen delay.  
Timing and color switching behave as expected.

### How to use task 1.1

Open project `Lab_1_1_LED` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)

---

## Task 1.2 – LED blinking using inverse bits (software toggle)

This task shows how to blink LEDs using **read–modify–write** instead of assigning constant `0`/`1`.

### Program logic

Inside the `for(;;)` loop:

- `Pin_Green_Write(~Pin_Green_Read());`
- `Pin_Blue_Write(~Pin_Blue_Read());`
- `CyDelay(100);`

So for each LED:

- `Read()` returns the current state (0 / 1),
- `~` inverts it,
- `Write(~Read())` toggles the LED.

### Result

Green and Blue LEDs blink with a **100 ms** period, purely via bit inversion.

### How to use task 1.2

Open project `Lab_1_2_LED` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)

---

## Task 1.3 – Hardware blinking with clock, divider and T-flip-flop

Here we move from software delays to **pure hardware blinking** using digital components.

### Hardware

- `Clock_1` at **1 kHz**
- **Frequency Divider** with division factor **500**
- **T Flip-Flop** (TFF)
- Outputs:
  - `Q` → Blue LED (`Pin_Blue`)
  - `Q` through inverter → Green LED (`Pin_Green`)
  - Red LED connected as before to `Pin_LED` (P2[6]) via 560 Ω

### Timing

- Clock: 1 kHz  
- Divider output: 1 kHz / 500 = **2 Hz**  
- TFF toggles on each rising edge → **1 Hz** at `Q`

So:

- Blue LED blinks at **1 Hz**
- Green LED blinks at **1 Hz**, but in **opposite phase**

No C code is required in the main loop (just an empty `for(;;)`).

### How to use task 1.3

Open project `Lab_1_3_LED` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)


### Result

BLUE and GREEN blink at 1 Hz in opposite phase, generated entirely by hardware.  
The CPU can sleep or do other tasks.

---

## Task 1 – Additional: LED fading using PWM (Green & Blue)

We created a smooth fade-in / fade-out effect using **PWM** for the Green and Blue LEDs.

### Hardware

- Two PWM components:
  - `PWM_GREEN`
  - `PWM_BLUE`
- Two clocks (`Clock_1`, `Clock_2`) at **10 kHz**, each driving a PWM.
- Connections:
  - `PWM_GREEN` output → `Pin_Green` (P3[6])
  - `PWM_BLUE` output → `Pin_Blue` (P3[7])
- Red LED (`Pin_LED_1` on P2[6]) kept as in previous tasks (not used here).

### Program logic

Pseudo-code:

1. Start both PWMs.
2. In a loop:
   - Increase `PWM_GREEN` duty from 0 → 255 (fade in), Blue = 0.
   - Decrease `PWM_GREEN` 255 → 0 (fade out).
   - Then do the same sequence for `PWM_BLUE`, while Green = 0.
   - Delay 10 ms between each duty update.

### How to use additional task 1

Open project `Lab_1_LED_PWM.cydsh` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)


### Result

- Green LED fades in and out smoothly, then Blue does the same.
- Fading speed is controlled by `CyDelay(10)` and the duty step.

---

## Task 2 – 4-digit 7-segment counter over SPI

We built a 4-digit counter using an external 7-segment display driven via **SPI** with **timer-based multiplexing** and buttons for control.

### Hardware

- `SPIM` – SPI Master, 16-bit transfers to the 7-segment driver (shift register / decoder).
- `Clock_1` (~800 Hz) + D-FF → source for **100 Hz** interrupt `isr_100Hz`.
- Buttons on shield:
  - `Pin_B1` (S1, P3[1]) – polled in main loop (slow down / simple debouncing).
  - `Pin_B2` (S2, P3[2]) – interrupt `isr_CNT_RST` (reset + switch to **0 → 9999** mode).
  - `Pin_B3` (S3, P3[3]) – interrupt `isr_CNT_NEG` (reset + switch to **0 → −999** mode).
- SPI pins:
  - `Pin_SPI_MOSI` – P0[5]
  - `Pin_SPI_SCLK` – P1[0]
  - `Pin_SPI_SS`   – P1[3]

### Data structures

- `indyk[4]` – 4 × 16-bit words; each holds digit enable + 7-segment pattern.
- `KOD7[10]` – segment codes for digits 0…9.
- `minus` – segment code for the minus sign.
- `counter` – current numeric value.
- `cnt_mode` – `0` for **0…9999**, `1` for **0…−999**.
- `floating_p_mode`, `point_pos` – used in the additional fixed-point mode.

### Display refresh ISR (`isr_100Hz`)

- Runs at ~100 Hz.
- Cycles `N_indyk` from 0 to 3.
- Sends `indyk[N_indyk]` via `SPIM_WriteTxData()` on each call.
- This multiplexes all 4 digits fast enough to avoid flicker.

### `out7seg(uint16 data, uint8 pos)`

Converts a number into segment patterns in `indyk[]`:

- For **cnt_mode = 0** (0…9999):
  - Places digit patterns in positions 0…3.
  - If `floating_p_mode == 1` and `pos` is valid, a decimal point is enabled at that position.
  - Digits to the left of the decimal point can be automatically padded with zeros.
- For **cnt_mode = 1** (0…−999):
  - Works similarly, but:
    - Only 3 digits are used for magnitude.
    - A minus sign is placed on the first available segment to the left of the digits.

### `rst_count()`

- Resets `counter` to 0 inside a critical section.
- Immediately updates the display:
  - integer mode → `out7seg(0, 0)`
  - fixed-point mode → `out7seg(0, point_pos)`

### Main logic

- Configure SPI + interrupts (`isr_100Hz`, `isr_CNT_RST`, `isr_CNT_NEG`).
- In the main loop:
  - Increment `counter`.
  - Reset when:
    - `cnt_mode = 0` and `counter > 9999`, or
    - `cnt_mode = 1` and `counter > 999`.
  - Call `out7seg(counter, point_pos)`.
  - Wait 3 ms.
  - If `Pin_B1` is pressed → additional delay (slow down counting).

### How to use task 2

Open project `Lab_2_7SEG.cydsh` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)


### Result

- In **0…9999 mode** (B2) the counter increases and wraps around to 0.
- In **0…−999 mode** (B3) the display shows a minus sign and counts as negative.
- Button B1 slows down the counter.
- Display is stable and readable thanks to the 100 Hz multiplexing.

---

### Task 3.1 – UART integer input and display

We extended the SPI-driven 7-segment display with UART control so that integers entered from a PC terminal are shown on the display.

#### Program logic

1. Characters from UART are accumulated into `rxBuf`.
2. When Enter is pressed:
   - The string is **null-terminated**.
   - Parsed with `sscanf(rxBuf, "%d", &value)`.
   - If parsing succeeds:
     - The value is **clamped to 0…9999** (out-of-range values are mapped to 0).
     - The number is displayed via `out7seg(value)`.
     - The board prints: `OK: value`.
   - If parsing fails:
     - The board prints: `Parse error`.
3. After processing, the board prompts again for the next integer.

### How to use task 3.1

Open project `Lab_3_1_UART_Transmitting.cydsh` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)


### How to use additional task 3

Open project `Lab_3_1_UART_additional.cydsh` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)

#### Result

- User can enter integers via the terminal and see them on the 7-segment display.
- Out-of-range values are handled safely.
- Display refresh remains smooth because multiplexing is done in the timer ISR, independent of UART speed.

---

## Task 3.2 – UART-based numeric input with editing (Backspace support)

In this task we implemented an interactive UART interface that allows the user to **type, edit and confirm** a number before displaying it on the 4-digit 7-segment indicator.

The goal is to make the PSoC board behave like a small text UI:

- user types a number in the terminal;
- the board echoes every character;
- user can delete digits with Backspace;
- only after pressing Enter the value is sent to the display.

---

### Hardware

Task 3.2 reuses the same display hardware as in Task 3.1 and adds four status LEDs:

- **UART** – communication with the PC terminal  
- **SPIM** – SPI Master for 4-digit 7-segment driver
- **Clock + D-Flip-Flop → `isr_100Hz`** – periodic interrupt for digit multiplexing
- **Discrete LEDs**:
  - `Pin_D1`, `Pin_D2`, `Pin_D3`, `Pin_D4` – initialized to `LED_OFF` (reserved for diagnostics)

Pin mapping (as in previous tasks):

- UART TX → `P1[4]`
- UART RX → `P1[5]`
- SPI MOSI → `P0[5]`  
- SPI SCLK → `P1[0]`  
- SPI SS   → `P1[3]`

The 7-segment display is driven by a 16-bit SPI word, using the `indyk[4]` buffer and `KOD7[10]` digit codes.  
The ISR `isr_100Hz` cycles through 4 digits and updates them via `SPIM_WriteTxData()`.

### How to use 3.2

Open project `Lab_3_2_UART_Led.cydsh` in PSoc Creator 4.4
Build the project
Connect the PSoC board to your computer
Program the device (Click `Program`)

## Overall Results

During this laboratory work we:

- Controlled basic LEDs using **GPIO** and explored direct writes and inverted toggling.
- Implemented **hardware-only LED blinking** using a clock, frequency divider and T-flip-flop.
- Used **PWM** for smooth LED brightness control and fading effects.
- Designed and drove a **4-digit 7-segment display** over **SPI** using timer-based multiplexing.
- Implemented multiple counting modes (**0…9999** and **0…−999**) with button-based control and support for fixed-point output.
- Integrated **UART communication** to receive integers and floating-point values from a PC terminal.
- Built an interactive UART interface with:
  - real-time input echo,
  - validation and length limits,
  - clear error messages,
  - Backspace-based editing support.

