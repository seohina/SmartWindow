<?php #저장된 사진을 불러오는 소스코드
 
	if($_SERVER['REQUEST_METHOD']=='GET'){
# safe테이블에 저장된 것 중에 최근값을 선택한다.
		$sql = "select * from safe ORDER BY time DESC limit 1";
# 위에 있는 conn.php에 접속한다.
		require "conn.php";
# mysql에 연결한다.
		$r = mysqli_query($conn,$sql);
		
		$result = mysqli_fetch_array($r);
		header('content-type: image/jpeg');
# blob변환된 사진을 디코딩하여 jpeg로 변환하여 이미지를 불러온다.
		echo base64_decode($result['image']);
		$row=mysqli_fetch_array($sql);
		mysqli_close($conn); # mysql 연결을 끊는다
# mysql에 연결되지 못하면		
	}else{
		echo "Error"; # error를 띄운다
	}
?>