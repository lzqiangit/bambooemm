/*
Navicat MySQL Data Transfer

Source Server         : lzq
Source Server Version : 50728
Source Host           : 127.0.0.1:3306
Source Database       : kvlist

Target Server Type    : MYSQL
Target Server Version : 50728
File Encoding         : 65001

Date: 2024-12-14 12:51:02
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for kv_list_10_5
-- ----------------------------
DROP TABLE IF EXISTS `random`;
CREATE TABLE `random` (
  `key` varchar(255) NOT NULL,
  `value` varchar(255) NOT NULL,
  `counter` int(255) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
