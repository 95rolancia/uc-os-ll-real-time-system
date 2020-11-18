# uc-os-ll-real-time-system
## 실행 환경
+ Visual Studio 2019
+ Micrium_Win32_Kernel

## practice 1. 상호 동기화하는 태스크 구현

![실시간 시스템 실습 1](https://user-images.githubusercontent.com/41102293/95830503-b93abf80-0d72-11eb-96b8-9a6f291ec7ec.png)

* TaskStart는 0~9 사이의 난수를 발생하여 Message Queue에 post한다.
* Task 1, 2, 3은 Message Queue의 값을 가져와 합산한다.
* Message Queue에 대한 접근에 Lock을 걸고, 사용 후 Lock을 푼다.
* TaskStart는 총 100번을 반복 수행하며, 임의의 키 입력을 통하여 계속 수행할 수 있도록 한다.
* TaskStart의 횟수는 모니터에 출력한다.
* 각 Task는 값을 누적한 후 Mailbox에 자신의 id와 누적값을 전송한다.
* TaskStart는 Mailbox를 읽어 메시지 내용에서 id와 누적값을 출력한다.
* 반복횟수가 100번이 넘거나 각 태스크의 누적값이 100이 넘으면 모든 ECB(Event Control Block)를 반환하고, 3개의 Task를 삭제 및 자신의 Task도 삭제하여 프로그램을 종료한다.
---
## practice 2. 간단한 가상의 경보 시스템 구현
![실시간 시스템 실습 2](https://user-images.githubusercontent.com/41102293/99492390-d2a5db80-29b0-11eb-94fa-3e74fc4efa07.jpg)
* 센서를 통해 외부의 환경 데이터를 아날로그로 입력 받음
* 특정 값이 임계치를 넘겼을 때 그 상황을 시간 정보와 함께 관리 서버에게 전송
