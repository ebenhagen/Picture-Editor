#include "widget.h"
#include "ui_widget.h"

#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::initMainWindow()
{
    this->setWindowTitle("OpenCV图片编辑器");
}

void Widget::imgShow() //显示图片
{
    //显示原图和原图的大小
    ui->lblOld->setPixmap(QPixmap::fromImage(m_OldQImageImg.scaled(ui->lblOld->size(), Qt::KeepAspectRatio)));
    ui->lblOldHeight->setText(QString("高度：%1").arg(m_OldQImageImg.height()));
    ui->lblOldWidth->setText(QString("宽度：%1").arg(m_OldQImageImg.width()));
    //显示修改后的图和图的大小
    ui->lblNew->setPixmap(QPixmap::fromImage(m_NewQImageImg.scaled(ui->lblNew->size(), Qt::KeepAspectRatio)));
    ui->letNewHeight->setText(QString::number(m_NewQImageImg.height()));
    ui->letNewWidth->setText(QString::number(m_NewQImageImg.width()));
}


void Widget::on_btnText_clicked() //文件按钮槽函数
{
    QString filePath = QFileDialog::getOpenFileName(this, "打开图片", "C:/Users/lenovo/Pictures", "(打开图片(*.png *.jpg *.bmp))");
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "文件打开失败！");
        return;
    }
    //文件存到Mat
    m_MatImg = imread(filePath.toLatin1().data());
    //初始化Mat新图
    m_NewMatImg = m_MatImg;
    //cvtColor
    cvtColor(m_MatImg, m_MatImg, COLOR_BGR2RGB);
    //QImage
    m_OldQImageImg = QImage(m_MatImg.data, m_MatImg.cols, m_MatImg.rows, m_MatImg.cols * m_MatImg.channels(), QImage::Format_RGB888);
    m_NewQImageImg = QImage(m_MatImg.data, m_MatImg.cols, m_MatImg.rows, m_MatImg.cols * m_MatImg.channels(), QImage::Format_RGB888);
    //显示
    imgShow();
}

void Widget::on_cbxImgDealChoice_currentTextChanged(const QString &arg1)
{
    if (m_MatImg.empty()) {
        return;
    }
    if (arg1 == "对比度" || arg1 == "亮度") {
        ui->horizontalSlider->setMaximum(99);
        ui->horizontalSlider->setMinimum(0);
    }
    else if (arg1 == "滤波") {
        ui->horizontalSlider->setMaximum(33);
        ui->horizontalSlider->setMinimum(0);
    }
    else if (arg1 == "二值化" || arg1 == "二值化（反）") {
        ui->horizontalSlider->setMaximum(255);
        ui->horizontalSlider->setMinimum(0);
    }
    else if (arg1 == "边缘检测canny" ) {
        imgProc_edge_detection(1);
    }
    else if (arg1 == "边缘检测laplacian") {
        imgProc_edge_detection(2);
    }
    else if (arg1 == "边缘检测sobel") {
        imgProc_edge_detection(3);
    }
    else {
        ui->horizontalSlider->setMaximum(99);
        ui->horizontalSlider->setMinimum(0);
    }
}



void Widget::on_horizontalSlider_sliderMoved(int position) //滑动条移动函数
{
    if (m_MatImg.empty()) {
        return;
    }
    switch(ui->cbxImgDealChoice->currentIndex()) { //根据combox的选择，分别调用不同的处理函数
        case 0 : {
            imgProc_contract_brightness(position / 33.3, 0);
            break;
        }
        case 1 : {
            imgProc_contract_brightness(1.0, position);
             break;
        }
        case 2 : {
             imgProc_filter_waves(position);
             break;
        }
        case 3 : {
             imgProc_binarization(position, 0);
             break;
        }
        case 4 : {
             imgProc_binarization(position, 1);
             break;
        }
        default : {
            break;
        }
    }


}


void Widget::on_horizontalSlider_valueChanged(int value)//滑动条值变化函数
{
    if (m_MatImg.empty()) {
            return;
    }
    switch(ui->cbxImgDealChoice->currentIndex()) {
        case 0 : {
                imgProc_contract_brightness(value / 33.3, 0);
              qDebug() << value;
                break;
        }
        case 1 : {
                imgProc_contract_brightness(1.0, value);
                  qDebug() << value;
                break;
        }
        case 2 : {
                imgProc_filter_waves(value);
                  qDebug() << value;
                break;
        }
        case 3 : {
                imgProc_binarization(value, 0);
                qDebug() << value;
                break;
        }
        case 4 : {
                imgProc_binarization(value, 1);
                qDebug() << value;
                break;
        }
        default : {
                break;
        }
    }
}

void Widget::on_btnSave_clicked() //“保存图片”槽函数
{
    if (m_MatImg.empty()) {
        QMessageBox::warning(this, "保存错误", "请先传入图片");
        return;
    }
    //6种保存类型
    QString imgFileType = "jpg(*.jpg);;png(*.png);;bmp(*.bmp);;jpeg(*.jpeg);;ico(*.ico);;All Files (*)";
    QString filePath = QFileDialog::getSaveFileName(this, "保存文件", QString(), imgFileType);
    if (!filePath.isEmpty()) {
        QString fileExtension = filePath.right(4);
        if (fileExtension == ".ico") { //.ico类型的用Qt的保存方式
            m_NewQImageImg.save(filePath);
        }
        else {                         //其他类型的用OpenCV的imwrite保存方式
            cvtColor(m_NewMatImg, m_NewMatImg, COLOR_RGB2BGR);
            cv::imwrite(filePath.toStdString(), m_NewMatImg);
            cvtColor(m_NewMatImg, m_NewMatImg, COLOR_BGR2RGB);  //注意图片颜色，不加这行在重复保存时会不正常
        }
    }
    else {
        QMessageBox::warning(this, "保存错误", "图片保存失败！");
    }
}

void Widget::imgProc_contract_brightness(float con, int bri) //“对比度”和“亮度”的处理函数
{
    Mat imgSrc = m_MatImg;                                       //获取原图
    Mat imgDst = Mat::zeros(imgSrc.size(), imgSrc.type());       //创建空白图像，相同的大小和类型
    imgSrc.convertTo(imgDst, -1, con, bri);                      //调灰度和亮度的函数
    m_NewQImageImg = QImage(imgDst.data, imgDst.cols, imgDst.rows, imgDst.cols * imgDst.channels(), QImage::Format_RGB888);
    m_NewMatImg = imgDst;
    imgShow();
}

void Widget::imgProc_filter_waves(int ker) //“滤波”的处理函数
{
    Mat imgSrc = m_MatImg;
    Mat imgDst = imgSrc.clone();
    if (ui->rBtnBlur->isChecked()) {
        for (int i = 1; i < ker; i = i + 2) blur(imgSrc, imgDst, Size(i, i), Point(-1, -1));           //均值滤波
    }
    else if (ui->rBtnGaussian->isChecked()) {
         for (int i = 1; i < ker; i = i + 2) GaussianBlur(imgSrc, imgDst, Size(i, i), 0, 0);            //高斯滤波
    }
    else if (ui->rBtnMedian->isChecked()) {
         for (int i = 1; i < ker; i = i + 2) medianBlur(imgSrc, imgDst, i);                             //中值滤波
    }
    else if (ui->rBtnBilateral->isChecked()){
         for (int i = 1; i < ker; i = i + 2) bilateralFilter(imgSrc, imgDst, i, i * 2, i / 2);          //双边滤波
    }
    m_NewQImageImg = QImage(imgDst.data, imgDst.cols, imgDst.rows, imgDst.cols * imgDst.channels(), QImage::Format_RGB888);
    m_NewMatImg = imgDst;
    imgShow();
}

void Widget::imgProc_binarization(double thresholdValue, int mode) //“二值化”处理函数
{
    Mat imgSrc = m_MatImg;
    Mat imgDst = Mat::zeros(m_MatImg.size(), m_MatImg.type());
    //灰度处理
    cvtColor(m_MatImg, imgDst, COLOR_BGR2GRAY);
    //Qt中QImage必须带三原色！
    cvtColor(imgDst, imgDst, COLOR_GRAY2BGR);

    int width = imgDst.cols * 3;
    int height = imgDst.rows;

    // 遍历图像像素
    int graya = 255, grayb = 0;            //阈值
    if (mode) std::swap(graya, grayb);     //mode传1则为反二值化
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int grayValue = imgDst.at<uchar>(i, j); // 获取灰度值
            // 将灰度值与滚动条值比较
            if (grayValue >= thresholdValue) {
                imgDst.at<uchar>(i, j) = graya; // 设置为灰度a
            }
            else {
                imgDst.at<uchar>(i, j) = grayb; // 设置为灰度b
            }
        }
    }
    m_NewQImageImg = QImage(imgDst.data, imgDst.cols, imgDst.rows, imgDst.cols * imgDst.channels(), QImage::Format_RGB888);
    m_NewMatImg = imgDst;
    imgShow();
}

void Widget::imgProc_changeSize_revolve(float ang, float sca){ //“缩放”与“旋转”处理函数
    Point2f srcMatrix[3];
    Point2f dstMatrix[3];
    Mat imgRot(2, 3, CV_32FC1);
    Mat imgSrc = m_MatImg;
    Mat imgDst;
    Point centerPoint = Point(imgSrc.cols / 2, imgSrc.rows / 2);    //计算原图像的中心点
    imgRot = getRotationMatrix2D(centerPoint, ang, sca);            //根据角度和缩放参数求得旋转矩阵
    warpAffine(imgSrc, imgDst, imgRot, imgSrc.size());              //执行旋转操作
    m_NewQImageImg = QImage(imgDst.data, imgDst.cols, imgDst.rows, imgDst.cols * imgDst.channels(), QImage::Format_RGB888);
    m_NewMatImg = imgDst;
    imgShow();
}


void Widget::on_hsrRevolve_sliderMoved(int position) //“旋转”滑动条
{
    if (m_MatImg.empty()) {
        return;
    }
    imgProc_changeSize_revolve(float(position - 360), ui->hsrChangeSize->value() / 100.0);
}


void Widget::on_hsrRevolve_valueChanged(int value)
{
    if (m_MatImg.empty()) {
        return;
    }
    imgProc_changeSize_revolve(float(value - 360), ui->hsrChangeSize->value() / 100.0);
}

void Widget::on_hsrChangeSize_sliderMoved(int position) //“缩放”滑动条
{
    if (m_MatImg.empty()) {
        return;
    }
    imgProc_changeSize_revolve(float(ui->hsrRevolve->value() - 360), position / 100.0);
}

void Widget::on_hsrChangeSize_valueChanged(int value)
{
    if (m_MatImg.empty()) {
        return;
    }
    imgProc_changeSize_revolve(float(ui->hsrRevolve->value() - 360), value / 100.0);
}


void Widget::imgProc_edge_detection(const int num) //“边缘检测”处理函数，参数对应三种不同的处理
{
    //针对的是灰度图
    Mat grayImage = m_MatImg;
    cvtColor(m_MatImg, grayImage, COLOR_BGR2GRAY);

    Mat imgDst = Mat::zeros(m_MatImg.size(), m_MatImg.type());

    if (num == 1) {
        //1.调用canny边缘检测
        Canny(grayImage, imgDst, 200, 1);
    }
    if (num == 2) {
        //2.拉普拉斯
        Laplacian(grayImage, imgDst, grayImage.depth());
    }
    if (num == 3) {
        //3.sobel计算xy方向的边缘检测图
        Mat sobel_x, sobel_y;
        Sobel(grayImage, sobel_x, CV_8U, 1, 0);
        Sobel(grayImage, sobel_y, CV_8U, 0, 1);
        //把xy方向加权平均法
        addWeighted(sobel_x, 0.5, sobel_y, 0.5, 0, imgDst);
    }
    //Qt中QImage必须带三原色！
    cvtColor(imgDst, imgDst, COLOR_GRAY2BGR);
    m_NewQImageImg = QImage(imgDst.data, imgDst.cols, imgDst.rows, imgDst.cols * imgDst.channels(), QImage::Format_RGB888);
    m_NewMatImg = imgDst;
    imgShow();

}


void Widget::on_letNewHeight_editingFinished() //设置新图的高度
{
    if (m_MatImg.empty()) {
        return;
    }
    int w = ui->letNewWidth->text().toInt();
    int h = ui->letNewHeight->text().toInt();
    if (ui->rBtnOriginalProportion->isChecked()){ //保持原比例
        w = (double)m_OldQImageImg.width() / (double)m_OldQImageImg.height() * h;
    }
    m_NewQImageImg = m_OldQImageImg.scaled(w, h);
    cv::resize(m_NewMatImg, m_NewMatImg, cv::Size(w, h));
    m_NewQImageImg = QImage(m_NewMatImg.data, m_NewMatImg.cols, m_NewMatImg.rows, m_NewMatImg.cols * m_NewMatImg.channels(), QImage::Format_RGB888);
    imgShow();
}


void Widget::on_letNewWidth_editingFinished() //设置新图的宽度
{
    if (m_MatImg.empty()) {
        return;
    }
    int w = ui->letNewWidth->text().toInt();
    int h = ui->letNewHeight->text().toInt();
    if (ui->rBtnOriginalProportion->isChecked()){ //保持原比例
        h = (double)m_OldQImageImg.height() /  (double)m_OldQImageImg.width() * w;
    }
    m_NewQImageImg = m_OldQImageImg.scaled(w, h);
    cv::resize(m_NewMatImg, m_NewMatImg, cv::Size(w, h));
    m_NewQImageImg = QImage(m_NewMatImg.data, m_NewMatImg.cols, m_NewMatImg.rows, m_NewMatImg.cols * m_NewMatImg.channels(), QImage::Format_RGB888);
    imgShow();
}


QString Widget::tobase64(const QImage &img, const QString &imgType) //“输出图片转base64”处理函数
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, imgType.toLocal8Bit().data());
    QString res = byteArray.toBase64();
    qDebug() << res.length();
    buffer.close();
    return res;
}

QImage Widget::toImage(const QString &base64str) //“base64转图片”处理函数
{
    QByteArray byteArray = QByteArray::fromBase64(base64str.toUtf8());
    QImage img;
    img.loadFromData(byteArray);
    return img;
}

void Widget::on_btnImg2Base_clicked()
{
    if (m_MatImg.empty()) {
        return;
    }
    ui->tbrBase64->setText(tobase64(m_NewQImageImg, ui->cbxImgTypeChoice->currentText()));
}

void Widget::on_btnBase2Img_clicked()
{
    if (ui->tbrBase64->toPlainText().isEmpty()) {
        QMessageBox::information(this, "保存错误", "Base64为空");
        return;
    }
    QString allType = "All Files (*)";
    QString filePath = QFileDialog::getSaveFileName(this, "另存为", "myImg." + ui->cbxImgTypeChoice->currentText(), allType);
    if (filePath.isEmpty()) {
        QMessageBox::information(this, "保存错误", "路径为空");
    }
    else {
        toImage(ui->tbrBase64->toPlainText()).save(filePath);
    }
}


