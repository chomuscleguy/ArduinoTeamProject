# 프로젝트 개요
[발표자료]  
https://www.miricanvas.com/v/14h3vj9
## 컨벤션
### 깃컨벤션
#### 커밋은 기능별로 구현이 되면 꼭 커밋하고 알려주셔야 합니다.
---
#### Develop Branch
- 모든 작업은 Main이 아닌 Develop이라는 보조 Branch를 생성하여 해당 브랜치의 하위에서 작업
- 제품이 완성 되었을 때만 Main에 Merge
---
#### Branch 생성 기준
ex ) LJH-GameManager-InitGame
이름-Class-기능
- 브랜치는 각 클래스 또는 더 작은 범위로 하여 해당 메서드 기능이 문제 없이 작동하면 커밋하여 프로젝트를 지속적으로 최신화 해야합니다.
---
#### Commit, PR, Merge
- 해당 작업을 수행 하실때는 꼭 팀원 전체에 공유가 되어야합니다.
- 해당 메세지를 받으신 팀원분들 전원은 꼭 브랜치를 최신화 해주셔야 합니다.
---
#### Commit Message
- Summary를 꼭 작성해주시기 바랍니다.
- Message는 해당 커밋에서 바뀐것 아니면, 해당 커밋을 받는 팀원들이 참고해야하는 참고사항을 꼭 기입해주세요
---
#### Summary [Head]
[💫 Feature] : 새로운 클래스 작성  
[🐞 BugFix] : 기존 코드의 버그를 수정  
[➕ Add] : 기존 기능에 추가적인 확장 기능을 제공  
[👨🏻‍🚒 HOTFIX] : 심각하거나 즉각 해결해야 하는 내용이 있을때  
### 코드컨벤션
https://docs.arduino.cc/learn/contributions/arduino-writing-style-guide/  
공식 도큐먼트 스타일 가이드 참고  
  
예외 사항  
- 도큐먼트에선 상수값 선언을 지양하고 있지만 수업때 계속 하던 방식으로 상수값 선언은 계속 진행
- 도큐먼트에선 포인터 사용을 지양하고 있지만 학습을 위해 포인터 사용에 제한을 두진 않음

## 핵심 기능
### 블루투스 통신
스마트폰 어플을 통해 블루투스로 데이터를 송신
### 어플은 유니티로 구현
https://github.com/chomuscleguy/ArduinoTeamProject/tree/cho_branch 유니티 어플리케이션이 있는 브런치  
안드로이드 스튜디오 API를 사용하여 만든 플러그인을 유니티 프로젝트에 임포트하여 구현  
플러그인 출처(https://github.com/bentalebahmed/BlueUnity)  
아두이노 프로젝트이기 때문에 많은 시간을 유니티 어플 제작에 쏟지는 않음  
### RC카 기능
서보모터를 통해 문 개폐  
릴레이 모듈을 통해 시동 ON/OFF  
진동 센서를 통한 충돌감지  
적외선 센터를 통해 장애물 감지  
피에조로 소리 출력  
DC모터와 모터드라이브를 통해 전후진  
## 하드웨어
### 사용부품
- 블루투스 hc-06
- 릴레이모듈 5V 릴레이 모듈 4CH
- 진동 SW-18010P 진동 충격감지 센서 모듈
- 모터드라이버 L298N
- 적외선 센서 SHARP GP2Y0A21YK0F 적외선 거리 측정 세션
- 부저
- 피에조 스피커
- SG-90 서보모터
- DC모터
- 건전지(18650 건전지, 6V)
- LED (흰색) * 2
- 아두이노 우노
- 브레드보드 * 2
### 회로도
![회로도](https://github.com/chomuscleguy/ArduinoTeamProject/blob/main/Image/KakaoTalk_20250414_092053239.png?raw=true)

## 소스코드  
디자인 패턴인 상태패턴을 사용하여 FSM을 구성

            IState  
              ↓  
          BaseState                        StateMachine  
        ↙           ↘              (상태머신을 컨트롤하는 함수가 위치)  
    IdleState     EngineState  


LED와 PIEZO부저 소리를 제어할 때 delay 사용을 자제하며, millis로 측정하여 동적으로 실행할 수 있도록 구성
