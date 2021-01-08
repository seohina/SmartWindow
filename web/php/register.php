<?php # 디바이스 토큰을 DB에 저장하는 소스코드
# 앱 최초 설치시 얻는 디바이스 토큰값을 안드로이드에서 가져온다.
   if(isset($_POST["Token"])){

      $token = $_POST["Token"];
# 데이터 베이스에 접속해서 토큰을 저장
      include_once 'config.php';
      $conn = mysqli_connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME);
# 가져온 토큰값을 users 테이블에 값을 저장한다.
      $query = "INSERT INTO users(Token) Values ('$token') ON DUPLICATE KEY UPDATE Token = '$token'; ";
      mysqli_query($conn, $query);
# mysql 연결을 끊는다.
      mysqli_close($conn);
   }
?>