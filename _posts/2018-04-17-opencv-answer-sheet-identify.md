---
layout: post
title: "基于 OpenCV 的 iOS 客户端答题卡识别算法"
date: 2018-04-17 21:13:00.000000000 +08:00
tags: OpenCV
---



最近计划学习一些图像处理方面的知识，第一时间想到了功能强大的 OpenCV Lib。

早在一年多前出来北京实习的时候，实习公司的一个短视频处理 App 在最初的技术选型的时候就将 OpenCV 作为重要解决方案之一。无奈的是，当初我是一个没毕业的 iOS 小菜鸡，初出茅庐又不懂的 C++，还要肩负起独立开发的大旗，实在是搞不懂也没时间搞这么高精尖的 Lib。在放弃 OpenCV Lib 以及尝试过 AVFoundation 框架后，因此项目最终选用了 GPUImage 来实现，当然这都是后话了。

这次的 OpenCV 的学习，一方面是为了弥补之前的技术空白，另一方面也是为了自己能在图像处理上有所了解，能站在巨人的肩膀上提升一下自身技术的维度。

### 目的及结果
本篇的写作目的是记录学习 OpenCV Lib 以及运用到答题卡识别的相关过程，并且也是对新学习知识点的梳理和重新组织。

本文的实现目的是：利用 OpenCV Lib 识别一张特定的答题卡照片，并且识别出学生填涂的选项。

##### 原图如下：

<img src="/Users/ios02/cuipengfei.cn/githubError.github.io/assets/post_images/2018/opencv_origin.jpg" 30%>

##### 实现结果如下：（红框内为识别结果）
<img src="/Users/ios02/cuipengfei.cn/githubError.github.io/assets/post_images/2018/opencv_result.png" 30%>


### 准备
考虑到 OpenCV 是基于 C/C++ 可跨平台的通用 Lib，为了降低学习成本，便将整个学习和实验集成到 iOS 的开发环境里了。前期要做如下几方面的准备工作：

1. 下载编译 OpenCV Lib，或者直接下载最新的 iOS OpenCV.framework 的 Release 版本；
2. 将自行编译或 Release 版 OpenCV.framework 导入 iOS 项目工程中；
3. 因为 OpenCV 中的 MIN 宏和 UIKit 的 MIN 宏有冲突，所以需要在 .pch 文件中，先定义 OpenCV 的头文件，否则会有编译错误；
4. 将需要混编 C++ 和 Objective-C 的文件后缀改为 **.mm**;
5. 为 UIImage 添加 Category，方便与OpenCV 图象格式的数据 cv::Mat 相互转换。
因这些繁琐的配置问题不是本文写作重点，而且网上不乏一些详细说明，推荐参考 [在MacOS和iOS系统中使用OpenCV](https://blog.devtang.com/2012/10/27/use-opencv-in-ios/) 一文，这里就不再赘述。

### 技术方案
需要说明的是，在学习 OpenCV 的基础知识时，无意间发现唐巧大神几年前写的 [猿题库iOS客户端的技术细节（二）：答题卡扫描算法](http://blog.devtang.com/2013/10/19/the-tech-detail-of-ape-client-2/) 一文。文中提到，在文章发布时相关的识别算法还在进行专利申请，并且在专利申请结束会披露算法细节，但是遗憾的是相关的算法细节并没有公开。不过万幸的是，唐巧大神提供了一套不错的解决方案，我本人的算法就是按照这个思路展开的，方案如下：

1. 图像预处理，压缩图像；
2. 将彩色图像转为灰度图像；
3. 二值化灰度图像，识别答题卡区域；
4. 透视变换，图像纠偏；
5. 答案区域 ROI 识别；
6. ROI 色值统计，标定答案。

## 未完待续...




-EOF-
