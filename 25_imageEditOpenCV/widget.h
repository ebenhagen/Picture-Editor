#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <opencv2/opencv.hpp>

using namespace cv;//opencv命名空间

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void initMainWindow();                          //界面初始化
    void imgProc_contract_brightness(float contrast, int brightness);   //处理图片
    void imgShow();                                 //显示图片

    void imgProc_filter_waves(int ker);
    void imgProc_binarization(double thresholdValue, int mode);
    void imgProc_changeSize_revolve(float ang, float sca);
    void imgProc_edge_detection(const int num);
    QString tobase64(const QImage &img, const QString &imgType);
    QImage toImage(const QString &base64str);


private:
    Mat m_MatImg;               //原图，Mat类型
    QImage m_OldQImageImg;      //原图，QImage类型      （图片大小修改）
    Mat m_NewMatImg;            //修改后的图，Mat类型    （保存图片）
    QImage m_NewQImageImg;      //修改后的图，QImage类型 （显示和修改）

private slots:
    void on_btnText_clicked();

    void on_horizontalSlider_sliderMoved(int position);

    void on_horizontalSlider_valueChanged(int value);

    void on_btnSave_clicked();

    void on_hsrChangeSize_sliderMoved(int position);

    void on_hsrChangeSize_valueChanged(int value);

    void on_hsrRevolve_sliderMoved(int position);

    void on_hsrRevolve_valueChanged(int value);

    void on_letNewHeight_editingFinished();

    void on_letNewWidth_editingFinished();

    void on_btnImg2Base_clicked();

    void on_btnBase2Img_clicked();

    void on_cbxImgDealChoice_currentTextChanged(const QString &arg1);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
