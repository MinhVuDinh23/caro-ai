# Bảng kết quả thực nghiệm (sinh tự động bởi caro_bench)

Điểm = (thắng + 0.5·hòa) / số ván, theo góc nhìn tác nhân A. T-H-B = thắng-hòa-thua của A.

## E1 - Đấu vòng tròn

| Nhóm | Tác nhân A | Đối thủ B | Ván | T-H-B | Tỉ lệ thắng | Điểm | ±KTC 95% | ms/nước A | ms/nước B | Số nước TB |
|---|---|---|---|---|---|---|---|---|---|---|
| cross | Random | MiniMax(d=1,AB,k=12) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.00 | 0.02 | 9.5 |
| cross | Random | MiniMax(d=2,AB,k=12) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.00 | 0.08 | 10.2 |
| cross | Random | MiniMax(d=3,AB,k=12) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.00 | 0.31 | 9.7 |
| cross | Random | MCTS(it=500,c=1.40,rand) | 100 | 2-0-98 | 2.0% | 0.020 | 0.027 | 0.00 | 2.99 | 24.1 |
| cross | Random | MCTS(it=500,c=1.40,heur) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.00 | 34.66 | 34.0 |
| cross | Random | MCTS(it=3000,c=1.40,rand) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.00 | 20.45 | 13.6 |
| cross | Random | MCTS(it=500,c=1.40,rand,fm) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.00 | 3.65 | 21.3 |
| cross | MiniMax(d=1,AB,k=12) | MiniMax(d=2,AB,k=12) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.01 | 0.07 | 10.5 |
| cross | MiniMax(d=1,AB,k=12) | MiniMax(d=3,AB,k=12) | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.02 | 0.34 | 18.5 |
| cross | MiniMax(d=1,AB,k=12) | MCTS(it=500,c=1.40,rand) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.02 | 3.22 | 9.9 |
| cross | MiniMax(d=1,AB,k=12) | MCTS(it=500,c=1.40,heur) | 100 | 96-0-4 | 96.0% | 0.960 | 0.038 | 0.02 | 81.19 | 9.7 |
| cross | MiniMax(d=1,AB,k=12) | MCTS(it=3000,c=1.40,rand) | 100 | 92-0-8 | 92.0% | 0.920 | 0.053 | 0.02 | 19.64 | 11.5 |
| cross | MiniMax(d=1,AB,k=12) | MCTS(it=500,c=1.40,rand,fm) | 100 | 96-0-4 | 96.0% | 0.960 | 0.038 | 0.02 | 3.43 | 11.2 |
| cross | MiniMax(d=2,AB,k=12) | MiniMax(d=3,AB,k=12) | 100 | 0-100-0 | 0.0% | 0.500 | 0.098 | 0.07 | 0.31 | 81.0 |
| cross | MiniMax(d=2,AB,k=12) | MCTS(it=500,c=1.40,rand) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.09 | 3.20 | 10.8 |
| cross | MiniMax(d=2,AB,k=12) | MCTS(it=500,c=1.40,heur) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.09 | 80.60 | 11.6 |
| cross | MiniMax(d=2,AB,k=12) | MCTS(it=3000,c=1.40,rand) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.10 | 20.25 | 12.5 |
| cross | MiniMax(d=2,AB,k=12) | MCTS(it=500,c=1.40,rand,fm) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.10 | 3.43 | 10.8 |
| cross | MiniMax(d=3,AB,k=12) | MCTS(it=500,c=1.40,rand) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.36 | 3.33 | 10.5 |
| cross | MiniMax(d=3,AB,k=12) | MCTS(it=500,c=1.40,heur) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.36 | 78.70 | 11.7 |
| cross | MiniMax(d=3,AB,k=12) | MCTS(it=3000,c=1.40,rand) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.37 | 19.81 | 12.3 |
| cross | MiniMax(d=3,AB,k=12) | MCTS(it=500,c=1.40,rand,fm) | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 0.35 | 3.24 | 10.2 |
| cross | MCTS(it=500,c=1.40,rand) | MCTS(it=500,c=1.40,heur) | 100 | 19-0-81 | 19.0% | 0.190 | 0.077 | 3.08 | 54.72 | 21.0 |
| cross | MCTS(it=500,c=1.40,rand) | MCTS(it=3000,c=1.40,rand) | 100 | 8-0-92 | 8.0% | 0.080 | 0.053 | 3.30 | 19.92 | 13.6 |
| cross | MCTS(it=500,c=1.40,rand) | MCTS(it=500,c=1.40,rand,fm) | 100 | 28-0-72 | 28.0% | 0.280 | 0.088 | 3.35 | 3.65 | 19.0 |
| cross | MCTS(it=500,c=1.40,heur) | MCTS(it=3000,c=1.40,rand) | 100 | 34-0-66 | 34.0% | 0.340 | 0.093 | 72.67 | 19.20 | 14.5 |
| cross | MCTS(it=500,c=1.40,heur) | MCTS(it=500,c=1.40,rand,fm) | 100 | 25-3-72 | 25.0% | 0.265 | 0.087 | 61.59 | 3.10 | 27.6 |
| cross | MCTS(it=3000,c=1.40,rand) | MCTS(it=500,c=1.40,rand,fm) | 100 | 78-0-22 | 78.0% | 0.780 | 0.081 | 20.31 | 3.53 | 13.6 |

| Tác nhân | Điểm trung bình | ms/nước trung bình |
|---|---|---|
| Random | 0.003 | 0.00 |
| MiniMax(d=1,AB,k=12) | 0.691 | 0.02 |
| MiniMax(d=2,AB,k=12) | 0.929 | 0.09 |
| MiniMax(d=3,AB,k=12) | 0.929 | 0.34 |
| MCTS(it=500,c=1.40,rand) | 0.219 | 3.21 |
| MCTS(it=500,c=1.40,heur) | 0.351 | 66.30 |
| MCTS(it=3000,c=1.40,rand) | 0.491 | 19.94 |
| MCTS(it=500,c=1.40,rand,fm) | 0.388 | 3.43 |

## E2 - Quét hằng số c

| Nhóm | Tác nhân A | Đối thủ B | Tham số | Ván | T-H-B | Tỉ lệ thắng | Điểm | ±KTC 95% | ms/nước A | ms/nước B | Số nước TB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| vs Random | MCTS(it=300,c=0.50,rand) | Random | c=0.5 | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 1.94 | 0.00 | 23.9 |
| vs Random | MCTS(it=300,c=1.00,rand) | Random | c=1.0 | 100 | 99-0-1 | 99.0% | 0.990 | 0.020 | 1.89 | 0.00 | 28.5 |
| vs Random | MCTS(it=300,c=1.40,rand) | Random | c=1.4 | 100 | 99-0-1 | 99.0% | 0.990 | 0.020 | 1.89 | 0.00 | 30.6 |
| vs Random | MCTS(it=300,c=2.00,rand) | Random | c=2.0 | 100 | 98-0-2 | 98.0% | 0.980 | 0.027 | 1.82 | 0.00 | 29.2 |
| vs MCTS(300,c=1.4) | MCTS(it=300,c=0.50,rand) | MCTS(it=300,c=1.40,rand) | c=0.5 | 200 | 131-0-69 | 65.5% | 0.655 | 0.066 | 1.98 | 2.00 | 20.9 |
| vs MCTS(300,c=1.4) | MCTS(it=300,c=1.00,rand) | MCTS(it=300,c=1.40,rand) | c=1.0 | 200 | 111-0-89 | 55.5% | 0.555 | 0.069 | 2.02 | 2.03 | 23.0 |
| vs MCTS(300,c=1.4) | MCTS(it=300,c=1.40,rand) | MCTS(it=300,c=1.40,rand) | c=1.4 | 200 | 87-0-113 | 43.5% | 0.435 | 0.069 | 1.92 | 1.91 | 22.5 |
| vs MCTS(300,c=1.4) | MCTS(it=300,c=2.00,rand) | MCTS(it=300,c=1.40,rand) | c=2.0 | 200 | 100-0-100 | 50.0% | 0.500 | 0.069 | 1.91 | 1.91 | 23.3 |
| vs MiniMax d=1 (it=10000) | MCTS(it=10000,c=0.50,rand) | MiniMax(d=1,AB,k=12) | c=0.5 | 100 | 50-2-48 | 50.0% | 0.510 | 0.098 | 62.68 | 0.03 | 20.2 |
| vs MiniMax d=1 (it=10000) | MCTS(it=10000,c=1.00,rand) | MiniMax(d=1,AB,k=12) | c=1.0 | 100 | 22-2-76 | 22.0% | 0.230 | 0.082 | 64.48 | 0.03 | 17.7 |
| vs MiniMax d=1 (it=10000) | MCTS(it=10000,c=1.40,rand) | MiniMax(d=1,AB,k=12) | c=1.4 | 100 | 30-3-67 | 30.0% | 0.315 | 0.091 | 65.05 | 0.03 | 19.2 |
| vs MiniMax d=1 (it=10000) | MCTS(it=10000,c=2.00,rand) | MiniMax(d=1,AB,k=12) | c=2.0 | 100 | 21-0-79 | 21.0% | 0.210 | 0.080 | 67.74 | 0.02 | 14.0 |

## E3 - Quét số iteration

| Nhóm | Tác nhân A | Đối thủ B | Tham số | Ván | T-H-B | Tỉ lệ thắng | Điểm | ±KTC 95% | ms/nước A | ms/nước B | Số nước TB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| vs Random | MCTS(it=100,c=1.40,rand) | Random | iterations=100 | 100 | 96-0-4 | 96.0% | 0.960 | 0.038 | 0.58 | 0.00 | 40.3 |
| vs Random | MCTS(it=300,c=1.40,rand) | Random | iterations=300 | 100 | 99-0-1 | 99.0% | 0.990 | 0.020 | 1.85 | 0.00 | 30.6 |
| vs Random | MCTS(it=500,c=1.40,rand) | Random | iterations=500 | 100 | 99-0-1 | 99.0% | 0.990 | 0.020 | 3.38 | 0.00 | 23.3 |
| vs Random | MCTS(it=1000,c=1.40,rand) | Random | iterations=1000 | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 6.79 | 0.00 | 19.2 |
| vs Random | MCTS(it=3000,c=1.40,rand) | Random | iterations=3000 | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 20.88 | 0.00 | 13.0 |
| vs Random | MCTS(it=10000,c=1.40,rand) | Random | iterations=10000 | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 70.26 | 0.00 | 10.8 |
| vs MCTS(300,c=1.4) | MCTS(it=100,c=1.40,rand) | MCTS(it=300,c=1.40,rand) | iterations=100 | 200 | 26-0-174 | 13.0% | 0.130 | 0.047 | 0.62 | 1.90 | 26.3 |
| vs MCTS(300,c=1.4) | MCTS(it=300,c=1.40,rand) | MCTS(it=300,c=1.40,rand) | iterations=300 | 200 | 87-0-113 | 43.5% | 0.435 | 0.069 | 1.93 | 1.93 | 22.5 |
| vs MCTS(300,c=1.4) | MCTS(it=500,c=1.40,rand) | MCTS(it=300,c=1.40,rand) | iterations=500 | 200 | 116-0-84 | 58.0% | 0.580 | 0.068 | 3.23 | 1.94 | 20.8 |
| vs MCTS(300,c=1.4) | MCTS(it=1000,c=1.40,rand) | MCTS(it=300,c=1.40,rand) | iterations=1000 | 200 | 157-0-43 | 78.5% | 0.785 | 0.057 | 6.75 | 2.02 | 17.5 |
| vs MCTS(300,c=1.4) | MCTS(it=3000,c=1.40,rand) | MCTS(it=300,c=1.40,rand) | iterations=3000 | 200 | 191-0-9 | 95.5% | 0.955 | 0.029 | 20.59 | 2.05 | 13.6 |
| vs MCTS(300,c=1.4) | MCTS(it=10000,c=1.40,rand) | MCTS(it=300,c=1.40,rand) | iterations=10000 | 200 | 200-0-0 | 100.0% | 1.000 | 0.000 | 69.16 | 2.04 | 10.9 |
| vs MiniMax d=1 | MCTS(it=100,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=100 | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.73 | 0.02 | 9.6 |
| vs MiniMax d=1 | MCTS(it=300,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=300 | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 1.98 | 0.02 | 9.7 |
| vs MiniMax d=1 | MCTS(it=500,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=500 | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 3.52 | 0.02 | 10.1 |
| vs MiniMax d=1 | MCTS(it=1000,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=1000 | 100 | 4-0-96 | 4.0% | 0.040 | 0.038 | 6.79 | 0.02 | 10.3 |
| vs MiniMax d=1 | MCTS(it=3000,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=3000 | 100 | 7-0-93 | 7.0% | 0.070 | 0.050 | 20.05 | 0.02 | 12.3 |
| vs MiniMax d=1 | MCTS(it=10000,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=10000 | 100 | 30-3-67 | 30.0% | 0.315 | 0.091 | 66.82 | 0.03 | 19.2 |

## E5 - Rollout heuristic vs ngẫu nhiên (Bài 2 - Hướng A)

| Nhóm | Tác nhân A | Đối thủ B | Tham số | Ván | T-H-B | Tỉ lệ thắng | Điểm | ±KTC 95% | ms/nước A | ms/nước B | Số nước TB |
|---|---|---|---|---|---|---|---|---|---|---|---|
| heur vs rand (cùng iteration) | MCTS(it=100,c=1.40,heur) | MCTS(it=100,c=1.40,rand) | iterations=100 | 200 | 194-0-6 | 97.0% | 0.970 | 0.024 | 12.25 | 0.63 | 30.4 |
| heur vs rand (cùng iteration) | MCTS(it=300,c=1.40,heur) | MCTS(it=300,c=1.40,rand) | iterations=300 | 200 | 165-0-35 | 82.5% | 0.825 | 0.053 | 34.89 | 1.90 | 22.2 |
| heur vs rand (cùng iteration) | MCTS(it=500,c=1.40,heur) | MCTS(it=500,c=1.40,rand) | iterations=500 | 200 | 160-0-40 | 80.0% | 0.800 | 0.055 | 57.41 | 3.17 | 20.8 |
| heur vs rand 20x iteration (cùng thời gian) | MCTS(it=100,c=1.40,heur) | MCTS(it=2000,c=1.40,rand) | iterations=100 | 100 | 12-0-88 | 12.0% | 0.120 | 0.064 | 15.66 | 13.42 | 15.3 |
| heur vs rand 20x iteration (cùng thời gian) | MCTS(it=300,c=1.40,heur) | MCTS(it=6000,c=1.40,rand) | iterations=300 | 100 | 10-0-90 | 10.0% | 0.100 | 0.059 | 50.23 | 42.04 | 11.9 |
| heur vs rand 20x iteration (cùng thời gian) | MCTS(it=500,c=1.40,heur) | MCTS(it=10000,c=1.40,rand) | iterations=500 | 100 | 10-0-90 | 10.0% | 0.100 | 0.059 | 82.17 | 68.58 | 11.2 |
| vs Random/rand | MCTS(it=100,c=1.40,rand) | Random | iterations=100 | 100 | 96-0-4 | 96.0% | 0.960 | 0.038 | 0.55 | 0.00 | 40.3 |
| vs Random/rand | MCTS(it=300,c=1.40,rand) | Random | iterations=300 | 100 | 99-0-1 | 99.0% | 0.990 | 0.020 | 1.80 | 0.00 | 30.6 |
| vs Random/rand | MCTS(it=500,c=1.40,rand) | Random | iterations=500 | 100 | 99-0-1 | 99.0% | 0.990 | 0.020 | 3.33 | 0.00 | 23.3 |
| vs Random/heur | MCTS(it=100,c=1.40,heur) | Random | iterations=100 | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 11.19 | 0.00 | 39.1 |
| vs Random/heur | MCTS(it=300,c=1.40,heur) | Random | iterations=300 | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 26.04 | 0.00 | 34.8 |
| vs Random/heur | MCTS(it=500,c=1.40,heur) | Random | iterations=500 | 100 | 100-0-0 | 100.0% | 1.000 | 0.000 | 37.69 | 0.00 | 33.8 |
| vs MiniMax d=1/rand | MCTS(it=100,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=100 | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 0.65 | 0.02 | 9.6 |
| vs MiniMax d=1/rand | MCTS(it=300,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=300 | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 1.93 | 0.02 | 9.7 |
| vs MiniMax d=1/rand | MCTS(it=500,c=1.40,rand) | MiniMax(d=1,AB,k=12) | iterations=500 | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 3.40 | 0.02 | 10.1 |
| vs MiniMax d=1/heur | MCTS(it=100,c=1.40,heur) | MiniMax(d=1,AB,k=12) | iterations=100 | 100 | 0-0-100 | 0.0% | 0.000 | 0.000 | 15.66 | 0.02 | 9.9 |
| vs MiniMax d=1/heur | MCTS(it=300,c=1.40,heur) | MiniMax(d=1,AB,k=12) | iterations=300 | 100 | 4-0-96 | 4.0% | 0.040 | 0.038 | 51.21 | 0.02 | 9.8 |
| vs MiniMax d=1/heur | MCTS(it=500,c=1.40,heur) | MiniMax(d=1,AB,k=12) | iterations=500 | 100 | 3-0-97 | 3.0% | 0.030 | 0.033 | 87.45 | 0.02 | 9.8 |

## E4 - MiniMax có / không cắt tỉa Alpha-Beta

| Thế cờ | d | top-k | Nút AB | Nút không cắt | Giảm | Lần cắt | ms AB | ms không cắt | Cùng nước | Cùng giá trị |
|---|---|---|---|---|---|---|---|---|---|---|
| P1 khai cuoc | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.012 | 0.011 | có | có |
| P1 khai cuoc | 2 | 12 | 38 | 157 | 75.8% | 11 | 0.056 | 0.140 | có | có |
| P1 khai cuoc | 3 | 12 | 192 | 1885 | 89.8% | 22 | 0.259 | 2.158 | có | có |
| P1 khai cuoc | 4 | 12 | 610 | 22621 | 97.3% | 187 | 1.218 | 26.281 | có | có |
| P1 khai cuoc | 1 | tất cả | 33 | 33 | 0.0% | 0 | 0.022 | 0.021 | có | có |
| P1 khai cuoc | 2 | tất cả | 102 | 1284 | 92.1% | 31 | 0.141 | 1.021 | có | có |
| P1 khai cuoc | 3 | tất cả | 1413 | 56704 | 97.5% | 62 | 1.387 | 47.648 | có | có |
| P2 dau van | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.012 | 0.011 | có | có |
| P2 dau van | 2 | 12 | 47 | 157 | 70.1% | 10 | 0.069 | 0.145 | có | có |
| P2 dau van | 3 | 12 | 213 | 1885 | 88.7% | 32 | 0.317 | 2.089 | có | có |
| P2 dau van | 4 | 12 | 692 | 22621 | 96.9% | 184 | 1.430 | 24.722 | có | có |
| P2 dau van | 1 | tất cả | 51 | 51 | 0.0% | 0 | 0.036 | 0.034 | có | có |
| P2 dau van | 2 | tất cả | 204 | 2688 | 92.4% | 48 | 0.293 | 2.297 | có | có |
| P2 dau van | 3 | tất cả | 3011 | 147574 | 98.0% | 151 | 3.228 | 134.835 | có | có |
| P3 X co 3 mo | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.012 | 0.011 | có | có |
| P3 X co 3 mo | 2 | 12 | 36 | 157 | 77.1% | 11 | 0.062 | 0.147 | có | có |
| P3 X co 3 mo | 3 | 12 | 191 | 1885 | 89.9% | 22 | 0.259 | 2.182 | có | có |
| P3 X co 3 mo | 4 | 12 | 333 | 21829 | 98.5% | 164 | 0.899 | 25.119 | có | có |
| P3 X co 3 mo | 1 | tất cả | 53 | 53 | 0.0% | 0 | 0.037 | 0.036 | có | có |
| P3 X co 3 mo | 2 | tất cả | 157 | 2866 | 94.5% | 51 | 0.265 | 2.342 | có | có |
| P3 X co 3 mo | 3 | tất cả | 3052 | 160044 | 98.1% | 103 | 2.734 | 127.317 | có | có |
| P4 O phai chan 4 | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.011 | 0.011 | có | có |
| P4 O phai chan 4 | 2 | 12 | 36 | 157 | 77.1% | 11 | 0.051 | 0.129 | có | có |
| P4 O phai chan 4 | 3 | 12 | 61 | 1753 | 96.5% | 22 | 0.104 | 1.995 | có | có |
| P4 O phai chan 4 | 4 | 12 | 227 | 20905 | 98.9% | 32 | 0.325 | 21.755 | có | có |
| P4 O phai chan 4 | 1 | tất cả | 55 | 55 | 0.0% | 0 | 0.036 | 0.036 | có | có |
| P4 O phai chan 4 | 2 | tất cả | 167 | 3057 | 94.5% | 53 | 0.232 | 2.409 | có | có |
| P4 O phai chan 4 | 3 | tất cả | 286 | 170883 | 99.8% | 111 | 0.602 | 136.616 | có | có |
| P5 X thang ngay | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.010 | 0.009 | có | có |
| P5 X thang ngay | 2 | 12 | 23 | 133 | 82.7% | 10 | 0.042 | 0.114 | có | có |
| P5 X thang ngay | 3 | 12 | 143 | 1573 | 90.9% | 10 | 0.140 | 1.588 | có | có |
| P5 X thang ngay | 4 | 12 | 253 | 16213 | 98.4% | 120 | 0.551 | 18.746 | có | có |
| P5 X thang ngay | 1 | tất cả | 44 | 44 | 0.0% | 0 | 0.029 | 0.028 | có | có |
| P5 X thang ngay | 2 | tất cả | 85 | 1937 | 95.6% | 41 | 0.158 | 1.497 | có | có |
| P5 X thang ngay | 3 | tất cả | 1967 | 94614 | 97.9% | 41 | 1.673 | 75.005 | có | có |
| P6 O chan 3 mo | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.012 | 0.011 | có | có |
| P6 O chan 3 mo | 2 | 12 | 36 | 157 | 77.1% | 11 | 0.065 | 0.147 | có | có |
| P6 O chan 3 mo | 3 | 12 | 217 | 1885 | 88.5% | 22 | 0.304 | 2.308 | có | có |
| P6 O chan 3 mo | 4 | 12 | 555 | 22621 | 97.5% | 187 | 1.221 | 27.228 | có | có |
| P6 O chan 3 mo | 1 | tất cả | 61 | 61 | 0.0% | 0 | 0.043 | 0.042 | có | có |
| P6 O chan 3 mo | 2 | tất cả | 180 | 3717 | 95.2% | 59 | 0.342 | 3.001 | có | có |
| P6 O chan 3 mo | 3 | tất cả | 4023 | 229149 | 98.2% | 118 | 3.659 | 182.557 | có | có |
| P7 giua van day quan | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.013 | 0.012 | có | có |
| P7 giua van day quan | 2 | 12 | 36 | 157 | 77.1% | 11 | 0.067 | 0.149 | có | có |
| P7 giua van day quan | 3 | 12 | 191 | 1885 | 89.9% | 22 | 0.239 | 2.014 | có | có |
| P7 giua van day quan | 4 | 12 | 333 | 21565 | 98.5% | 164 | 0.959 | 25.052 | có | có |
| P7 giua van day quan | 1 | tất cả | 61 | 61 | 0.0% | 0 | 0.046 | 0.046 | có | có |
| P7 giua van day quan | 2 | tất cả | 180 | 3674 | 95.1% | 59 | 0.335 | 3.137 | có | có |
| P7 giua van day quan | 3 | tất cả | 3838 | 221528 | 98.3% | 118 | 3.604 | 175.526 | có | có |
| P8 tranh chap goc | 1 | 12 | 13 | 13 | 0.0% | 0 | 0.009 | 0.009 | có | có |
| P8 tranh chap goc | 2 | 12 | 36 | 157 | 77.1% | 11 | 0.044 | 0.120 | có | có |
| P8 tranh chap goc | 3 | 12 | 195 | 1885 | 89.7% | 22 | 0.212 | 1.845 | có | có |
| P8 tranh chap goc | 4 | 12 | 988 | 22621 | 95.6% | 255 | 1.729 | 22.821 | có | có |
| P8 tranh chap goc | 1 | tất cả | 28 | 28 | 0.0% | 0 | 0.018 | 0.018 | có | có |
| P8 tranh chap goc | 2 | tất cả | 81 | 887 | 90.9% | 26 | 0.100 | 0.669 | có | có |
| P8 tranh chap goc | 3 | tất cả | 992 | 32387 | 96.9% | 52 | 1.021 | 27.206 | có | có |

