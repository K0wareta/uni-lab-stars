#include "cmath"
#include <QPainter>
#include <fstream>
#include <iostream>
#ifndef STAR_H
#define STAR_H
using namespace std;

const int dim = 2;
      int numStars;
const int borderMassC = 10;
const double G = 6.67408e-11;
      double systemRadius, distConnect, dt = 100000; // dt в 10 раз (*)
const double massSun   = 1.98892e30,
             massJup   = 1898.6e24,
             massUran  = 86.832e24,
             massEarth = 5.9742e24,
             massVenus = 4.867e24;
const double borderMass[] = {borderMassC*massEarth, borderMassC*massUran, borderMassC*massJup, borderMassC*massSun};
const QColor colStar[] = {Qt::cyan, Qt::red, Qt::magenta, Qt::yellow, Qt::white}; //Поменял цвет планет на красный (2)
const int nColor = sizeof(colStar) / sizeof(colStar[0]);

class star{
public:
    static int starCounter;
    double x[dim];
    double v[dim];
    double m;
    double f[dim];
    double impulse[dim];
    double kineticEnergy;
    QColor col;
    star(double *coord, double *speed, double mass);
    ~star(){starCounter--;}
    void operator += (const star &dobject);
};
int star::starCounter = 0;

void star::operator += (const star &dobject){                         //Перегрузка оператора для слияния двух объектов (3)
    double tmpM = m + dobject.m, tmpX[dim], tmpV[dim];
    for(int k = 0; k < dim; ++k){
        tmpX[k] = (x[k] * m + dobject.x[k] * dobject.m)/tmpM;
        tmpV[k] = (v[k] * m + dobject.v[k] * dobject.m)/tmpM;
    }
    m=tmpM;
    delete &dobject;
}

star::star(double *coord, double *speed, double mass){
     m = mass;
    for(int k = 0; k < dim; ++k){
        x[k] = coord[k];
        v[k] = speed[k];
        double vResult = sqrt(speed[0]*speed[0]+speed[1]*speed[1]);
        impulse[k]=speed[k]*mass;
        kineticEnergy=vResult*vResult*m/2;
    }
    col = colStar[nColor-1];  // не годится, если будут объединения объектов, функция намного лучше
    for(int i = 0; i < nColor-1; ++i){
        if(m <= borderMass[i]){
            col = colStar[i];
            break;
        }
    }
    starCounter++;
}
class galaxy{
public:
    int sliderChange = dt;
    int num;
    double massSum =  massSun;
    double impulseSum;
    double momentSum;
    double kineticSum;
    star **stars;
    friend ostream& operator<<(ostream &out, const galaxy *galaxy1){                //Перегразка оператора вывода (5)
        out<<"id\tmass\t\t\tx\t\ty\t\n";
        for (int i = 0; i < galaxy1->num; i++){
            if (galaxy1->stars[i]){
                out<<i<<"\t"<<galaxy1->stars[i]->m<<"\t"<<galaxy1->stars[i]->x[0]<<"\t"<<galaxy1->stars[i]->x[1]<<endl;
            }
        }
    }
    galaxy(int n = numStars):num(n){
        stars = new star*[num];

        double x1[dim] = {0}, v1[dim] = {0};
        stars[0] = new star(x1, v1, massSun); // самый массивный объект в начале координат
        double rad;
        for(int i = 1; i < num; ++i){
            rad = 0;
            double R = rand() * systemRadius / RAND_MAX,
            fi  = (2 * M_PI * rand()) / RAND_MAX,
            theta = (M_PI * rand()) / RAND_MAX;
            x1[0] = R  * cos(fi);
            x1[1] = R  * sin(fi);
            if(dim == 3){
                x1[0] *= sin(theta);
                x1[1] *= sin(theta);
                x1[3] = R * cos(theta);
            }
            for(int k = 0; k < dim; ++k){
                rad += x1[k] * x1[k];
            }
// вторая космическая скорость
            double absV = sqrt(G * stars[0]->m / sqrt(rad)), alpha = (2 * M_PI * rand()) / RAND_MAX;
//если размерность 3, нужен еще один угол как для координат(два угла годятся и для плоскости, желающие могут сделать)
//            v1[0] = absV * cos(alpha);
//            v1[1] = absV * sin(alpha);
            v1[0] =  absV * sin(fi);
            v1[1] = -absV * cos(fi); // скорость направлена вдоль окружности с центром в начале координат
            stars[i] = new star(x1, v1, massEarth / num * (6 * i));
            massSum += stars[i]->m;
            impulseSum += sqrt((stars[i]->impulse[0]*stars[i]->impulse[0])+(stars[i]->impulse[1]*stars[i]->impulse[1]));
        }
    };

    ~galaxy(){delete[] stars;};
    double border[dim];
    void move(){
        double dist;
        double dCoord[dim];
        for(int i = 0; i < num; ++i){ // force nullification
            for(int k = 0; k < dim; ++k){
                if(stars[i]){
                    stars[i]->f[k] = 0;
                }
            }
        }
        for(int i = 0; i < num; i++){
            if(stars[i]){
                for(int j = i + 1; j < num; j++){
                    if(i != j && stars[j]){
                        dist = 0;
                        for(int k = 0; k < dim; ++k){
                            dCoord[k] = stars[i]->x[k] - stars[j]->x[k];
                            dist += dCoord[k] * dCoord[k];
                        }
                        if(sqrt(dist) < distConnect){
                            *stars[i]+=*stars[j];
                            stars[j] = nullptr;

                            for(int r = 0; r < nColor - 1; r++){
                                if(stars[i]->m <= borderMass[r]){
                                    stars[i]->col = colStar[r];
                                    break;
                                 }
                            }
                        }
                    }
                }
            }
        }
        for(int i = 0; i < num; i++){
            if(stars[i]){
                for(int j = i + 1; j < num; j++){
                    if(i != j && stars[j]){
                        dist = 0;
                        for(int k = 0; k < dim; ++k){
                            dCoord[k] = stars[i]->x[k] - stars[j]->x[k];
                            dist += dCoord[k] * dCoord[k];
                        }
                        // dist = sqrt(dist); // для знаменателя пока квадрат, потом возьмем корень
                        for(int k = 0; k < dim; ++k){
                            double tmp = G * stars[i]->m * stars[j]->m / dist;
                            stars[i]->f[k] -= tmp * dCoord[k] / sqrt(dist);
                            stars[j]->f[k] += tmp * dCoord[k] / sqrt(dist);
                        }
                    }
                }
            }
        }
        double tempImp = 0;
        double tempKinet = 0;
        for(int i = 0; i < num; ++i){
            if(stars[i]){
                for(int k = 0; k < dim; ++k){
                    stars[i]->v[k] += sliderChange * stars[i]->f[k] / stars[i]->m; //можно не делить на массу, а выше суммировать ускорение
                    stars[i]->impulse[k] = stars[i]->v[k]*stars[i]->m;
                }
                double peremen = sqrt((stars[i]->impulse[0]*stars[i]->impulse[0])+(stars[i]->impulse[1]*stars[i]->impulse[1]));
                tempImp += peremen;
                tempKinet += peremen*peremen/(stars[i]->m*2);
                for(int k = 0; k < dim; ++k){
                    stars[i]->x[k] += sliderChange * stars[i]->v[k];
                }
            }
        }
        impulseSum = tempImp;
        kineticSum = tempKinet;
    }
};


#endif // STAR_H
