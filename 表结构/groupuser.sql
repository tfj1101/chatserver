/*
 Navicat Premium Dump SQL

 Source Server         : Ubuntu
 Source Server Type    : MySQL
 Source Server Version : 80043 (8.0.43-0ubuntu0.24.04.1)
 Source Host           : 192.168.155.140:3306
 Source Schema         : chat

 Target Server Type    : MySQL
 Target Server Version : 80043 (8.0.43-0ubuntu0.24.04.1)
 File Encoding         : 65001

 Date: 17/11/2025 16:55:39
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for groupuser
-- ----------------------------
DROP TABLE IF EXISTS `groupuser`;
CREATE TABLE `groupuser`  (
  `groupid` int NOT NULL,
  `userid` int NOT NULL,
  `grouprole` enum('creator','manager','normal') CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL DEFAULT 'normal',
  PRIMARY KEY (`groupid`, `userid`) USING BTREE,
  INDEX `userid`(`userid` ASC) USING BTREE,
  CONSTRAINT `groupuser_ibfk_1` FOREIGN KEY (`groupid`) REFERENCES `allgroup` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `groupuser_ibfk_2` FOREIGN KEY (`userid`) REFERENCES `user` (`id`) ON DELETE CASCADE ON UPDATE RESTRICT
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
