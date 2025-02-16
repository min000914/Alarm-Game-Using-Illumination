# 조도 기반 알람 게임

## 프로젝트 개요
ATMEGA-128을 통해 조도 센서를 활용한 알람 시스템을 구현했습니다. 주변의 밝기를 측정하여 임계값을 초과하면 알람이 작동합니다. 사용자는 3번의 퀴즈를 풀어야 알람을 해제할 수 있습니다.

![Image](https://github.com/user-attachments/assets/05caa52e-2a0b-40a8-908d-f5e8a0b2b7be)


## 주요 기능
### 1. 조도 감지
- 조도 센서(ADC)를 통해 주변 밝기를 측정합니다.
- 측정값이 임계값(`CDS_VALUE = 871`) 이상이면 알람이 활성화됩니다.

### 2. 퀴즈 시스템
- 0부터 7까지의 숫자를 랜덤으로 생성합니다.
- 생성된 숫자는 LED 패턴과 버저음을 통해 사용자에게 제시됩니다.
- 사용자는 버튼을 눌러 정답을 맞추고 점수를 획득해야 합니다.

### 3. 피드백 및 제어
- **LED**: 퀴즈 문제와 결과를 표시합니다.
- **버저**: 문제 숫자를 음계로 출력합니다.
- **FND(7-Segment Display)**: 현재 점수를 표시합니다.

### 4. 알람 해제
- 사용자가 3번의 퀴즈를 모두 맞추면 알람이 해제됩니다.
- 알람 해제 시 LED와 버저가 꺼지며 시스템이 종료됩니다.

## 시스템 구성
### 1. 하드웨어
- **조도 센서(LDR)**: 주변 밝기 측정
- **LED**: 퀴즈 번호와 결과 표시
- **버저**: 퀴즈 번호를 음계로 출력
- **FND**: 현재 점수 표시
- **스위치**: 사용자 입력

### 2. 소프트웨어
- **ucos-II**: 실시간 운영체제를 사용하여 멀티태스킹 구현
- **이벤트 플래그와 메시지 큐**: 작업 간 동기화 및 데이터 전달

## 작동 원리
1. 조도 센서가 주변 밝기를 측정하여 임계값 이상일 경우 알람이 작동합니다.
2. 퀴즈가 시작되며, 랜덤한 숫자가 LED와 버저를 통해 사용자에게 제시됩니다.
3. 사용자는 버튼을 눌러 정답을 맞추고, 3점을 달성하면 알람이 해제됩니다.

## 시연 영상 및 문서
- **시연 영상**: [YouTube 링크](https://youtube.com/shorts/yNjEl-dJqmE)
- **보고서**: [프로젝트 보고서 링크](https://github.com/min000914/Alarm-Game-Using-Illumination/blob/main/embedded_project.pdf)

## 참고 사항
- 이 시스템은 AVR 마이크로컨트롤러와 ucos-II RTOS를 기반으로 설계되었습니다.
- 자세한 코드와 동작 원리는 첨부된 보고서를 참고하세요.
