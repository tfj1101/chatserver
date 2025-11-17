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

 Date: 17/11/2025 16:55:45
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for offlineMessage
-- ----------------------------
DROP TABLE IF EXISTS `offlineMessage`;
CREATE TABLE `offlineMessage`  (
  `userid` int NOT NULL,
  `message` varchar(500) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_0900_ai_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
