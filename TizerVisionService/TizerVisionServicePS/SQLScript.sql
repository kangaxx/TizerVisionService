CREATE TABLE TB_BURR_COUNT_LOG (
  id int NOT NULL AUTO_INCREMENT,
  LOW_BURR_NUMBER int NOT NULL,
  MID_BURR_NUMBER int NOT NULL,
  HIGH_BURR_NUMBER int NOT NULL,
  UPH_BURR_NUMBER int NOT NULL,
  TOP_BURR_NUMBER int NOT NULL,
  BURR_TYPE char(20) NOT NULL,
  LOG_TIME char(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=10 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

CREATE TABLE TB_BURR_COUNT_DETAIL (
  id int NOT NULL AUTO_INCREMENT,
  BURR_1 float NOT NULL,
  BURR_2 float NOT NULL,
  BURR_3 float NOT NULL,
  BURR_4 float NOT NULL,
  BURR_5 float NOT NULL,
  BURR_TYPE char(20) NOT NULL,
  LOG_TIME char(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

