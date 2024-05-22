# Shear Sort 并行实现

中科大的并行策略，但是不适用于我们的情况，并非shearsort：[点击查看](https://wenku.baidu.com/view/83fd796858fafab069dc02ec?fr=xiongzhanghao&bfetype=new&_wkts_=1715902069200&needWelcomeRecommand=1)

这是一个很好的参考，用MPI_Alltoall：[点击查看](https://github.com/moalitoali/Parallelized-Shearsort/blob/main/Report.pdf)

### 申请资源

在 Snowy 上申请资源：

```bash
interactive -M snowy -A uppmax2024-2-9 -n 30 -t 04:10:00
```

### 加载软件环境

```bash
module load gcc/12.2.0 openmpi/4.1.4
```

### 环境报错

```bash
export OMPI_MCA_btl_openib_allow_ib=1
```

### 编译代码

```bash
make shearshort
```

### 执行程序

```bash
mpirun -n 16 ./shearsort <matrix_size> <input_file> <output_file>

mpirun -n 1 ./shearsort 5000 /home/lish6
557/local/src/PDP/ShearSort/file/input5000.txt result.txt
```

其中，`<matrix_size>` 是矩阵的大小，`<input_file>` 是输入数据文件的路径，`<output_file>` 是排序后的输出文件路径。