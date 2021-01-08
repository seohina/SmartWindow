<?php #로그인 소스 코드
require "conn.php";
# 데이터를 POST방식으로 전달한다
$user_name = $_POST["user_name"];
# 입력받은 user_name과 DB에 있는 user_name이 같은 값을 선택한다.
$mysql_qry = "select * from user where user_name like '$user_name';";
# mysql에 접속한다.
$result = mysqli_query($conn ,$mysql_qry);
if(mysqli_num_rows($result) > 0) {
# 값이 일치하면 login success
echo "login success";
}
 # 값이 일치하지 않으면 login not success
else { 
echo "login not success";
}
 
?>