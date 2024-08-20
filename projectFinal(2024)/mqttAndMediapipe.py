import cv2
import mediapipe as mp
import paho.mqtt.client as mqtt
import time
import sys

face_area_now = None

# Mediapipe 초기화
mp_face_mesh = mp.solutions.face_mesh
face_mesh = mp_face_mesh.FaceMesh(min_detection_confidence=0.5, min_tracking_confidence=0.5)
mp_drawing = mp.solutions.drawing_utils

# MQTT 설정
topic = "dahun_test"
broker = "mqtt.eclipseprojects.io"
port = 1883
client = mqtt.Client()

try:
    client.connect(broker, port)
except Exception as e:
    print(f"Failed to connect to MQTT broker: {e}")
    sys.exit(1)  # 프로그램 종료

client.loop_start()

# 웹캠 열기
cap = cv2.VideoCapture(0)
direction_list = ["a", "d", "g"]

try:
    while cap.isOpened():
        success, image = cap.read()
        if not success:
            print("Ignoring empty camera frame.")
            continue

        image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        image.flags.writeable = False
        results = face_mesh.process(image)
        image.flags.writeable = True
        image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

        if results.multi_face_landmarks:
            for face_landmarks in results.multi_face_landmarks:
                mp_drawing.draw_landmarks(
                    image=image,
                    landmark_list=face_landmarks,
                    connections=mp_face_mesh.FACEMESH_TESSELATION,
                    landmark_drawing_spec=None,
                    connection_drawing_spec=mp_drawing.DrawingSpec(color=(0, 255, 0), thickness=1, circle_radius=1))

                h, w, _ = image.shape
                landmarks = [(int(lm.x * w), int(lm.y * h)) for lm in face_landmarks.landmark]

                x_min = min(landmarks, key=lambda x: x[0])[0]
                y_min = min(landmarks, key=lambda x: x[1])[1]
                x_max = max(landmarks, key=lambda x: x[0])[0]
                y_max = max(landmarks, key=lambda x: x[1])[1]

                cv2.rectangle(image, (x_min, y_min), (x_max, y_max), (255, 0, 0), 2)

                face_width = x_max - x_min
                face_height = y_max - y_min
                face_area = face_width * face_height
                face_area = (face_area / 10000) / 2

                if face_area_now is None:
                    face_area_now = face_area

                cv2.putText(image, f'Area: {face_area}', (x_min, y_min),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 2, cv2.LINE_AA)

                if len(face_landmarks.landmark) > 1:
                    left_pupil_x = int(face_landmarks.landmark[1].x * image.shape[1])
                    left_pupil_y = int(face_landmarks.landmark[1].y * image.shape[0])
                    cv2.circle(image, (left_pupil_x, left_pupil_y), 5, (0, 255, 255), -1)
                    cv2.putText(image, f'Left Pupil: ({left_pupil_x}, {left_pupil_y})', (left_pupil_x, left_pupil_y - 10),
                                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 2, cv2.LINE_AA)
                    print(face_area)
                    print(face_area_now)

                    if 4 > face_area > 2:
                        if left_pupil_x < 350:
                            direction_list[0] = "a"
                        elif left_pupil_x > 400:
                            direction_list[0] = "c"
                        else:
                            direction_list[0] = "b"

                        if left_pupil_y > 280:
                            direction_list[1] = "d"
                        elif left_pupil_y < 100:
                            direction_list[1] = "f"
                        else:
                            direction_list[1] = "e"
                    else:
                        if face_area < face_area_now:
                            direction_list[2] = "g"  #멀어짐 - 가까워지게 해야됌
                        elif face_area > face_area_now:  #가까워짐 - 멀어지게 해야됌
                            direction_list[2] = "i"
                        else:                           #정지
                            direction_list[2] = "h"

                    face_area_now = face_area
                    

                direction_payload = ''.join(direction_list)
                client.publish(topic, direction_payload)
                print(f"Published '{direction_payload}' to the topic '{topic}'")
                
                # 0.25초 대기
                time.sleep(0.25)
        else:
            direction_list = ["b", "e", "h"]
            direction_payload = ''.join(direction_list)
            client.publish(topic, direction_payload)
            print(f"Published '{direction_payload}' to the topic '{topic}'")
            
            # 0.25초 대기
            time.sleep(0.25)

        cv2.imshow('MediaPipe FaceMesh', image)

        if cv2.waitKey(5) & 0xFF == 27:
            break

except KeyboardInterrupt:
    print("Stopped by user")

finally:
    client.loop_stop()
    client.disconnect()
    cap.release()
    cv2.destroyAllWindows()
