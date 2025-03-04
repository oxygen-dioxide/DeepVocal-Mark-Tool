#include "auplot.h"
#include "ui_auplot.h"

AuPlot::AuPlot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AuPlot)
{
    ui->setupUi(this);
    ui->splitter->setChildrenCollapsible(false);
    connect(athr,&AnaThread::lock,this,&AuPlot::lock);
    connect(athr,&AnaThread::unlock,this,&AuPlot::unlock);
    connect(athr,&AnaThread::out,this,&AuPlot::draw);
    connect(athr,&AnaThread::tip,this,&AuPlot::tip);
    connect(lthr,&LoaThread::lock,this,&AuPlot::lock);
    connect(lthr,&LoaThread::unlock,this,&AuPlot::unlock);
    connect(lthr,&LoaThread::out,this,&AuPlot::draw);
    connect(lthr,&LoaThread::tip,this,&AuPlot::tip);
    connect(sthr,&SvaThread::num,ui->stip,&STip::setnum);
    connect(sthr,&SvaThread::num,this,&AuPlot::onsvanum);
    connect(ui->waveview,&Waveview::hschanged,this,&AuPlot::onhschanged);
    connect(ui->waveplot,&Waveplot::hschanged,this,&AuPlot::onhschanged);
    connect(ui->specplot,&Specplot::hschanged,this,&AuPlot::onhschanged);
    connect(ui->tool,&ToolButtons::control,this,&AuPlot::onbuttonchanged);
    connect(ui->tool,&ToolButtons::plotplay,this,&AuPlot::plotplay);
    connect(ui->waveplot,&Waveplot::xon,ui->specplot,&Specplot::setxon);
    connect(ui->specplot,&Specplot::xon,ui->waveplot,&Waveplot::setxon);
    connect(ui->waveplot,&Waveplot::xmoved,ui->specplot,&Specplot::setxw);
    connect(ui->specplot,&Specplot::xmoved,ui->waveplot,&Waveplot::setxw);
    connect(&se,&QSoundEffect::playingChanged,this,&AuPlot::onsestop);
    connect(ui->waveplot,&Waveplot::changesp,this,&AuPlot::onspchanged);
    connect(ui->specplot,&Specplot::changesp,this,&AuPlot::onspchanged);
    connect(ui->waveplot,&Waveplot::pmove,this,&AuPlot::onpmove);
    connect(ui->specplot,&Specplot::pmove,this,&AuPlot::onpmove);
    connect(ui->waveplot,&Waveplot::point,this,&AuPlot::onpoint);
    connect(ui->specplot,&Specplot::point,this,&AuPlot::onpoint);
    connect(ui->waveplot,&Waveplot::prefresh,this,&AuPlot::onprefresh);
    connect(ui->specplot,&Specplot::prefresh,this,&AuPlot::onprefresh);
    connect(ui->specplot,&Specplot::prefresh,this,&AuPlot::onprefresh);
    connect(ui->tool,&ToolButtons::linechanged,this,&AuPlot::onlinechanged);
    connect(ui->tool,&ToolButtons::saved,this,&AuPlot::onsaved);
    connect(ui->tool,&ToolButtons::autoed,this,&AuPlot::autoed);
    //lock();
    this->tip("无数据!");
    this->lock();
    lthr->setPT(&this->pixtemp);
    sthr->setpix(&this->pixtemp);
}

AuPlot::~AuPlot()
{
    ld->close();
    delete ld;
    if(se.isPlaying()){
        se.stop();
    }
    if(athr->isRunning()){
        athr->terminate();
        athr->wait();
        athr->rele();
    }
    delete athr;
    if(lthr->isRunning()){
        lthr->terminate();
        lthr->wait();
        lthr->rele();
    }
    delete lthr;
    if(sthr->isRunning()){
        sthr->terminate();
        sthr->wait();
    }
    delete sthr;
    delete ui;
}

void AuPlot::resizeEvent(QResizeEvent *event)
{
    ui->tool->resize(width(),0.05*height());
    ui->tool->move(0,0);
    ui->waveview->resize(width(),0.05*height());
    ui->waveview->move(0,0.05*height());
    ui->splitter->resize(width(),0.9*height());
    ui->splitter->move(0,0.1*height());
    ui->waveplot->setMinimumHeight(0.2*height());
    ui->specplot->setMinimumHeight(0.2*height());
    ld->resize(width(),height());
    ld->move(0,0);
    ld->raise();
    ui->stip->resize(0.15*width(),0.1*height());
    ui->stip->move(width()-0.15*width()-0.015*width(),height()-0.1*height()-0.01*height());
    ui->stip->raise();
    QWidget::resizeEvent(event);
}

void AuPlot::WaveAnalyse(QString filepath,bool repaint)
{
    if(se.isPlaying()){
        se.stop();
    }

    if(repaint){
        QWaveHandle wave(filepath);
        if(wave.channels==1&&wave.samplespersec==44100&&wave.wavedataL->size()>=2*global_sets::perp&&wave.wavedataL->size()<=global_sets::maxlength){
            if(filepath!=this->filepath){
                this->setfourlines(this->fourlines);
            }
            this->filepath=filepath;

            athr->setwave(&wave);

            athr->start();
        }else{
            QMessageBox::warning(this,"错误!","无法读取波形文件!");
        }

    }else{



        if(pixtemp.check(filepath)){
            if(pixtemp.checkmd5(filepath)){
                if(filepath!=this->filepath){
                    this->setfourlines(this->fourlines);
                }
                lthr->setfilepath(filepath);
                lthr->start();
            }else{
                QWaveHandle wave(filepath);

                if(wave.channels==1&&wave.samplespersec==44100&&wave.wavedataL->size()>=2*global_sets::perp&&wave.wavedataL->size()<=global_sets::maxlength){

                    if(filepath!=this->filepath){
                        this->setfourlines(this->fourlines);
                    }
                    athr->setwave(&wave);

                    athr->start();
                }else{
                    QMessageBox::warning(this,"错误!","无法读取波形文件!");
                }
            }

        }else{
            QWaveHandle wave(filepath);

            if(wave.channels==1&&wave.samplespersec==44100&&wave.wavedataL->size()>=2*global_sets::perp&&wave.wavedataL->size()<=global_sets::maxlength){

                if(filepath!=this->filepath){
                    this->setfourlines(this->fourlines);
                }
                athr->setwave(&wave);

                athr->start();
            }else{
                QMessageBox::warning(this,"错误!","无法读取波形文件!");
            }
        }
        this->filepath=filepath;

    }

    this->onhschanged(0,1);

}

void AuPlot::lock()
{
    ld->resize(width(),height());
    ld->move(0,0);
    ld->raise();
    ld->show();
    lockf=true;
    emit locked();
}

void AuPlot::unlock()
{
    ld->hide();
    lockf=false;
    emit unlocked();
}

void AuPlot::draw(QVector<double> wavepixv,QVector<double> wavepixp,QVector<QVector<double>> specpix,QVector<double> v1,QVector<double> v2,QVector<bool> v4,bool save)
{
    ui->waveview->setpix(wavepixv);
    ui->waveplot->setpix(wavepixp);
    ui->waveplot->setv(v1,v2);
    ui->specplot->setv(v4);
    ui->specplot->setpix(specpix,wavepixv.size());

    this->onhschanged(0,1);
    this->onspchanged(false,0,0);

    if(save){
        sthr->set(filepath,wavepixv,wavepixp,specpix,v1,v2,v4);
        sthr->start();
    }

}

void AuPlot::tip(QString msg)
{
    ld->setmsg(msg);
}

bool AuPlot::islock()
{
    return lockf;
}

void AuPlot::onhschanged(double hs,double he)
{
    if(he-hs>=0.001){
        ui->waveview->seths(hs,he);
        ui->waveplot->seths(hs,he);
        ui->specplot->seths(hs,he);
        this->hs=hs;
        this->he=he;
    }

}

void AuPlot::onbuttonchanged(int code)
{
    ui->specplot->changemode(code);
    ui->waveplot->changemode(code);

    switch (code) {
    case -1:
        this->pixtemp.clear();
        this->WaveAnalyse(filepath,true);
        break;
    case 0:
        this->WaveAnalyse(filepath,true);
        break;


    case 7:

        break;
    case 8:


        break;
    default:

        break;
    }
}

void AuPlot::plotplay(bool play)
{
    if(selected){
        if(play){
            QWaveHandle *wavetemp=new QWaveHandle;
            QWaveHandle *wave=new QWaveHandle(filepath);
            QByteArray BA;


                    for(int i=ssp*wave->wavedataL->size();i<sep*wave->wavedataL->size();i++){
                        if(i<wave->wavedataL->size()){
                            wavetemp->wavedataL->append(wave->wavedataL->at(i));
                        }
                    }

                    wavetemp->WriteFile(QCoreApplication::applicationDirPath()+"/temp/play.wav");

                    delete wavetemp;
                    delete wave;
                    if(se.isPlaying()){
                        se.stop();
                    }
                    se.setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath()+"/temp/play.wav"));
                    se.setVolume(1);
                    se.setLoopCount(0);

                    se.play();

        }else{
            if(se.isPlaying()){
                se.stop();
            }

        }
    }

}

void AuPlot::onsestop()
{
    if(!se.isPlaying()){
        se.stop();
        se.setSource(QUrl());
        ui->tool->setplay(false);
        QFile file(QCoreApplication::applicationDirPath()+"/temp/play.wav");
        file.remove();
    }
}

void AuPlot::onspchanged(bool selected,double ssp,double sep)
{
    if(!se.isPlaying()){
        if(!selected){
            ui->waveplot->setselected(false,ssp,sep);
            ui->specplot->setselected(false,ssp,sep);
            ui->waveview->setselected(false,ssp,sep);
            ui->tool->setcanplay(false);
            this->selected=false;
            this->ssp=ssp;
            this->sep=sep;
        }else{
            if(ssp==sep){
                ui->waveplot->setselected(false,ssp,sep);
                ui->specplot->setselected(false,ssp,sep);
                ui->waveview->setselected(false,ssp,sep);
                ui->tool->setcanplay(false);
                this->selected=false;
                this->ssp=ssp;
                this->sep=sep;
            }else{
                ui->waveplot->setselected(true,ssp,sep);
                ui->specplot->setselected(true,ssp,sep);
                ui->waveview->setselected(true,ssp,sep);
                ui->tool->setcanplay(true);
                this->selected=true;
                this->ssp=ssp;
                this->sep=sep;
            }
        }
    }

}

void AuPlot::onpmove(double px)
{
    switch (this->lineid) {
    case 1:
        ui->specplot->setlines(px,l2,l3,l4);
        ui->waveplot->setlines(px,l2,l3,l4);
        break;
    case 2:
        ui->specplot->setlines(l1,px,l3,l4);
        ui->waveplot->setlines(l1,px,l3,l4);
        break;
    case 3:
        ui->specplot->setlines(l1,l2,px,l4);
        ui->waveplot->setlines(l1,l2,px,l4);
        break;
    case 4:
        ui->specplot->setlines(l1,l2,l3,px);
        ui->waveplot->setlines(l1,l2,l3,px);
        break;
    default:
        break;
    }
}

void AuPlot::onpoint(double px)
{
    switch (this->lineid) {
    case 1:
        ui->specplot->setlines(px,l2,l3,l4);
        ui->waveplot->setlines(px,l2,l3,l4);
        ui->waveview->setlines(px,l2,l3,l4);
        this->l1=px;
        break;
    case 2:
        ui->specplot->setlines(l1,px,l3,l4);
        ui->waveplot->setlines(l1,px,l3,l4);
        ui->waveview->setlines(l1,px,l3,l4);
        this->l2=px;
        break;
    case 3:
        ui->specplot->setlines(l1,l2,px,l4);
        ui->waveplot->setlines(l1,l2,px,l4);
        ui->waveview->setlines(l1,l2,px,l4);
        this->l3=px;
        break;
    case 4:
        ui->specplot->setlines(l1,l2,l3,px);
        ui->waveplot->setlines(l1,l2,l3,px);
        ui->waveview->setlines(l1,l2,l3,px);
        this->l4=px;
        break;
    default:
        break;
    }
    if(fourlines){
        if(l4>l3&&l3>l2&&l2>l1){
            ui->tool->setcansave(true);
        }else{
            ui->tool->setcansave(false);
        }
    }else{
        if(l2>l1){
            ui->tool->setcansave(true);
        }else{
            ui->tool->setcansave(false);
        }
    }

    ui->tool->wannachangeline();
}

void AuPlot::onprefresh()
{
    ui->specplot->setlines(l1,l2,l3,l4);
    ui->waveplot->setlines(l1,l2,l3,l4);
    ui->waveview->setlines(l1,l2,l3,l4);
}

void AuPlot::onlinechanged(int id)
{
    this->lineid=id;
    ui->specplot->setlines(l1,l2,l3,l4);
    ui->waveplot->setlines(l1,l2,l3,l4);
    ui->waveview->setlines(l1,l2,l3,l4);
}

void AuPlot::onsaved()
{
    if(pixtemp.checkmd5(filepath)){
        emit marks(l1,l2,l3,l4);
    }else{
        QMessageBox::warning(this,"错误!","检测到波形文件改动，请重新分析后再次尝试保存!");
    }
}

void AuPlot::setfourlines(bool fourlines)
{
    this->fourlines=fourlines;
    ui->tool->setfourlines(fourlines);
    ui->specplot->setfourlines(fourlines);
    ui->waveplot->setfourlines(fourlines);
    this->l1=0;
    this->l2=0;
    this->l3=0;
    this->l4=0;
    ui->specplot->setlines(l1,l2,l3,l4);
    ui->waveplot->setlines(l1,l2,l3,l4);
    ui->waveview->setlines(l1,l2,l3,l4);
    if(fourlines){
        if(l4>l3&&l3>l2&&l2>l1){
            ui->tool->setcansave(true);
        }else{
            ui->tool->setcansave(false);
        }
    }else{
        if(l2>l1){
            ui->tool->setcansave(true);
        }else{
            ui->tool->setcansave(false);
        }
    }
}

void AuPlot::setlines(double l1,double l2,double l3,double l4)
{
    this->l1=l1;
    this->l2=l2;
    this->l3=l3;
    this->l4=l4;
    ui->specplot->setlines(l1,l2,l3,l4);
    ui->waveplot->setlines(l1,l2,l3,l4);
    ui->waveview->setlines(l1,l2,l3,l4);
    if(fourlines){
        if(l4>l3&&l3>l2&&l2>l1){
            ui->tool->setcansave(true);
        }else{
            ui->tool->setcansave(false);
        }
    }else{
        if(l2>l1){
            ui->tool->setcansave(true);
        }else{
            ui->tool->setcansave(false);
        }
    }
}

void AuPlot::onsvanum(int num)
{
    if(num==100){
        emit unlocked();
        ui->tool->setlocked(false);
        checktemps();
    }else if(num==0){
        emit locked();
        ui->tool->setlocked(true);
    }else{

    }
}

void AuPlot::checktemps()
{
    double tsizen=pixtemp.size();
    if(tsizen>tsizeg){
        QString nots="缓存文件总大小已达到";
        if(tsizeg>=1024){
            nots+=QString::asprintf("%.2fGB",(double)((double)tsizen/(double)1024));
        }else{
            nots+=QString::asprintf("%.2fMB",tsizen);
        }
        nots+="，这将增加设备储存空间负担。\n是否要清除缓存？";
        QMessageBox::StandardButton result=QMessageBox::warning(this,"缓存容量",nots,QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No),QMessageBox::No);
        if(result==QMessageBox::Yes){
            tsizeg=global_sets::tempsgate;
            this->pixtemp.clear();
            this->WaveAnalyse(filepath,true);
        }else{
            tsizeg+=global_sets::tempsstep;
        }
    }
}
