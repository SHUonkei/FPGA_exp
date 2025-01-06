```mermaid
flowchart
  A[main()] --> B[srand42]
  B --> C[state_INIT]
  C --> D[mode_VS_MODE]
  D --> E[x_pos_0, y_pos_3]
  E --> F[point1_0, point2_0]
  F --> G[seed_1]
  G --> H[x_dir_1, y_dir_0]
  H --> I[max_delay_0, min_delay_3]
  I --> J[toggle_speed_0]
  J --> K[racket1_3, racket2_3]
  K --> L[cnt_0, delay_cnt_0, global_timer_0]
  L --> M{state}
  M -->|INIT| N[lcd_init]
  N --> O[state_OPENING]
  M -->|OPENING| P[state_MENU]
  M -->|MENU| Q[start]
  Q --> R[state_PLAY]
  M -->|PLAY| S[play]
  S --> T[state_RESULT]
  M -->|RESULT| U[result]
  U --> V[state_ENDING]
  M -->|ENDING| W[state_OPENING]

  %% start() function
  Q --> Q1[btn_check_0] --> Q2[mode_VS_MODE]
  Q --> Q3[btn_check_1] --> Q4[mode_VS_CPU]
  Q --> Q5[btn_check_2] --> Q6[state_PLAY]

  %% result() function
  U --> U1[btn_check_2] --> U2[point1_0, point2_0] --> U3[state_PLAY]

  %% play() function
  S --> S1[point1_0, point2_0]
  S1 --> S2{condition}
  S2 -->|point1_5 or point2_5| T
  S2 -->|x_pos_0 and kypd_scan_0x7| S3[led_blink]
  S2 -->|x_pos_11 and kypd_scan_0xB| S4[led_blink]
  S2 -->|btn_check_1| T

  %% interrupt_handler()
  IH --> IH1[state_INIT]
  IH1 --> IH2[do_nothing]
  IH --> IH3[state_OPENING] --> IH4[cnt_0, delay_min_delay]
  IH --> IH5[state_MENU] --> IH6[show_start]
  IH --> IH7[state_PLAY && mode_VS_MODE]
  IH7 --> IH8[delay_cnt++]
  IH8 --> IH9[update_x_pos, y_pos]
  IH9 --> IH10[handle_ball_collisions]
  IH10 --> IH11[handle_input_rackets]
  IH --> IH12[state_PLAY && mode_VS_CPU]
  IH12 --> IH13[CPU_movement]
  IH --> IH14[state_RESULT] --> IH15[show_result]
  IH --> IH16[state_ENDING] --> IH17[do_nothing]

  %% LCD Functions
  LCD1[lcd_init] --> LCD2[lcd_pwr_on]
  LCD2 --> LCD3[lcd_cmd] --> LCD4[lcd_data]
  LCD4 --> LCD5[lcd_clear_vbuf]
  LCD5 --> LCD6[lcd_sync_vbuf]

  %% LED Functions
  LED1[led_set]
  LED1 --> LED2[delay_ms]
  LED1 --> LED3[led_blink]

  %% Buzzer Functions
  BZ1[play_buzzer_high] --> BZ2[play_buzzer_low] --> BZ3[stop_buzzer]
```