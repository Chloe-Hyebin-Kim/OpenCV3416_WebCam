#  **[Ball detection using OpenCV3.4.16]** 




<!-- 🔗[문제링크](https://www.acmicpc.net/problem/11404) -->







## Introduction

<!--[Sendbird](https://sendbird.com) provides the chat API and SDK for your app, enabling real-time communication among the users.-->

<!--- [**Chat Sample**](#chat-sample) has core chat features. Group channel and open channel are the two main channel types in which you can create various subtypes where users can send and receive messages. This sample is written with [Sendbird Chat SDK for Unreal](https://github.com/sendbird/sendbird-chat-sdk-unreal).-->

<br />


## Requirements

- `VisualStudio 2019` , [VisualStudio 2019 📥](https://visualstudio.microsoft.com/ko/vs/older-downloads/) 
- `OpenCV 3.4.16` [OpenCV 📥](https://opencv.org/releases/) 


## Supported Platforms

- `Windows10` ,
- `VisualStudio 2019`



## Issue

1. 공이 아닌 원형도 감지.
![image](https://github.com/user-attachments/assets/edb7e188-8fa6-4790-9a5e-727b41b43400)


2. 색상이 유사한 배경은 결합되어 감지가 안됨
![image](https://github.com/user-attachments/assets/8b50071d-661a-4325-8db7-a193effb0fce)
![image](https://github.com/user-attachments/assets/d2af7363-d66b-43e3-990b-a2dfd4e126de)

![image](https://github.com/user-attachments/assets/8d114052-279e-4244-b794-b2c47b2ee56e)
![image](https://github.com/user-attachments/assets/e0d0110b-532c-4486-95b1-91d9d47d321d)


3. 공 색깔에 따라 감지되는 수준이 다름
   ![image](https://github.com/user-attachments/assets/47aaeb3c-aca2-4417-a0c3-01ac59a255ec)


## Solution


1. Canny 엣지와 컬러 컴바인 마스크 활용
   ![image](https://github.com/user-attachments/assets/da74adae-4268-4688-a9dc-10d131b1dc5e)

2. ![image](https://github.com/user-attachments/assets/4f04f2bc-9888-4f11-b6ae-dfd1c13e4b44)

3. 변인통제 필요


## Result

![image](https://github.com/user-attachments/assets/56b69ec5-2c2c-4876-be17-46722fb93a81)


![image](https://github.com/user-attachments/assets/4680783f-1b77-4ccd-89cc-bcb99d3e89e9)


## How to Run the Sample
