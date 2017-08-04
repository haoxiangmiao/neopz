//
//  pzsandlerPV.cpp
//  PZ
//
//  Created by Diogo Cecilio on 9/3/13.
//  Modified by Thiago Quinelato on 20/04/17.
//
//

#include "TPZSandlerDimaggioPV.h"
#include "pzlog.h"

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("plasticity.poroelastoplastic"));
#endif

#ifdef LOG4CXX
static LoggerPtr loggerConvTest(Logger::getLogger("ConvTest"));
#endif

TPZSandlerDimaggioPV::TPZSandlerDimaggioPV() {
    ftol = 1.e-5;
}

TPZSandlerDimaggioPV::TPZSandlerDimaggioPV(const TPZSandlerDimaggioPV & copy) {
    fA = copy.fA;
    fB = copy.fB;
    fC = copy.fC;
    fD = copy.fD;
    fK = copy.fK;
    fG = copy.fG;
    fW = copy.fW;
    fR = copy.fR;
    fE = (9. * fK * fG) / (3. * fK + fG);
    fnu = ((3. * fK)-(2. * fG)) / (2 * (3. * fK + fG));
    fElasticResponse.SetUp(fE, fnu);
}

TPZSandlerDimaggioPV::TPZSandlerDimaggioPV(STATE A, STATE B, STATE C, STATE D, STATE K, STATE G, STATE W, STATE R) :
fA(A), fB(B), fC(C), fD(D), fW(W), fK(K), fR(R), fG(G) {
    fE = (9. * fK * fG) / (3. * fK + fG);
    fnu = ((3. * fK)-(2. * fG)) / (2 * (3. * fK + fG));
    fElasticResponse.SetUp(fE, fnu);
    ftol = 1.e-5;
}

TPZSandlerDimaggioPV::~TPZSandlerDimaggioPV() {

}

template <class T>
T TPZSandlerDimaggioPV::F(const T x) const {
    return fA - fC * exp(x * fB);
}

STATE TPZSandlerDimaggioPV::GetF(STATE x) const {
    return F(x);
}

template<class T>
T TPZSandlerDimaggioPV::X(T L) const {
    return (L - fR * F(L));
}

STATE TPZSandlerDimaggioPV::GetX(STATE k) {
    return X(k);
}

void TPZSandlerDimaggioPV::SetUp(STATE A, STATE B, STATE C, STATE D, STATE K, STATE G, STATE W, STATE R) {
    fA = A;
    fB = B;
    fC = C;
    fD = D;
    fK = K;
    fG = G;
    fW = W;
    fR = R;
    fE = (9. * fK * fG) / (3. * fK + fG);
    fnu = ((3. * fK)-(2. * fG)) / (2 * (3. * fK + fG));
    TPZElasticResponse ER;
    ER.SetUp(fE, fnu);
    fElasticResponse = ER;
}

void TPZSandlerDimaggioPV::SetElasticResponse(TPZElasticResponse &ER) {
    fElasticResponse = ER;
    fE = ER.E();
    fnu = ER.Poisson();
    fK = ER.K();
    fG = ER.G();
}

TPZElasticResponse TPZSandlerDimaggioPV::GetElasticResponse() const {
    return fElasticResponse;
}

void TPZSandlerDimaggioPV::Read(TPZStream &buf) {
    buf.Read(&fA);
    buf.Read(&fB);
    buf.Read(&fC);
    buf.Read(&fD);
    buf.Read(&fK);
    buf.Read(&fG);
    buf.Read(&fW);
    buf.Read(&fR);
    buf.Read(&fE);
    buf.Read(&fnu);
    fElasticResponse.Read(buf);
}

void TPZSandlerDimaggioPV::Write(TPZStream &buf) const {
    buf.Write(&fA);
    buf.Write(&fB);
    buf.Write(&fC);
    buf.Write(&fD);
    buf.Write(&fK);
    buf.Write(&fG);
    buf.Write(&fW);
    buf.Write(&fR);
    buf.Write(&fE);
    buf.Write(&fnu);
    fElasticResponse.Write(buf);
}

TPZElasticResponse TPZSandlerDimaggioPV::GetElasticResponse() {
    return fElasticResponse;
}

STATE TPZSandlerDimaggioPV::GetR() {
    return fR;
}

template<class T>
T TPZSandlerDimaggioPV::EpsEqX(T X) const {
    return (fW * (exp(fD * X) - 1));
}

template<class T>
T TPZSandlerDimaggioPV::EpsEqk(T k) const {
    return EpsEqX(X(k));
}

void TPZSandlerDimaggioPV::Firstk(STATE &epsp, STATE &k) const {
    STATE f, df, kn1, kn, resnorm, diff;
    unsigned int counter = 0;
    resnorm = std::numeric_limits<STATE>::max();
    kn = epsp; //chute inicial
    kn1 = kn;
    do {
        f = EpsEqk(kn) - epsp;
        df = fD * exp(fD * (kn - (fA - fC * exp(fB * kn)) * fR))*(1 + fB * fC * exp(fB * kn) * fR) * fW;
        //df=fD*exp(fD*(kn - fR*(fA - fC*exp(fB*kn) - kn*fPhi)))*fW*(1 - fR*(-(fB*fC*exp(fB*kn)) - fPhi));
        kn1 = kn - f / df;
        kn = kn1;
        diff = kn1 - kn;
        resnorm = sqrt(diff * diff);
        counter++;
    } while (resnorm > ftol && counter < 30);
    k = kn1;
}

template<class T>
T TPZSandlerDimaggioPV::ResLF2(const TPZVec<T> &pt, T theta, T beta, T k, STATE kprev) const {
    T I1tr = (pt[0])+(pt[1])+(pt[2]);
    T I1 = fR * F(k) * cos(theta) + k;
    T delepsp = EpsEqk(k) - EpsEqk(kprev);
    return (3. * fK * delepsp - (I1tr - I1));
}

template<class T>
T TPZSandlerDimaggioPV::ResLF2IJ(const TPZVec<T> &sigtrialIJ, T theta, T k, STATE kprev) const {
    T I1tr = sigtrialIJ[0];
    T I1 = fR * F(k) * cos(theta) + k;
    T delepsp = EpsEqk(k) - EpsEqk(kprev);
    return (3. * fK * delepsp - (I1tr - I1));
}

/// Compute the residual of the equation which defines the update of the damage variable

STATE TPZSandlerDimaggioPV::ResLF1(const TPZVec<STATE> &sigtrial, const TPZVec<STATE> &sigproj, STATE k, STATE kprev) const {
    STATE I1tr = sigtrial[0] + sigtrial[1] + sigtrial[2];
    STATE I1 = sigproj[0] + sigproj[1] + sigproj[2];
    STATE delepsp = EpsEqk(k) - EpsEqk(kprev);
    return 3. * fK * delepsp - (I1tr - I1);
}

/// Compute the derivative of the equation which determines the evolution of k
// the derivative are given in terms of k

STATE TPZSandlerDimaggioPV::DResLF1(const TPZVec<STATE> &sigtrial, const TPZVec<STATE> &sigproj, STATE k, STATE kprev) const {
    STATE expfBk = exp(fB * k);
    STATE dreskk = 3. * exp(fD * (-((fA - expfBk * fC) * fR) + k)) * fD * fK *
            (1. + expfBk * fB * fC * fR) * fW;
    return dreskk;
}


/// Compute the derivative of the equation which determines the evolution of k

void TPZSandlerDimaggioPV::DResLF2(const TPZVec<STATE> &pt, STATE theta, STATE beta, STATE k, STATE kprev, TPZVec<STATE> &dresl) const {
    STATE expfBk = exp(fB * k);
    STATE sintheta = sin(theta);
    STATE costheta = cos(theta);

    STATE dreskk = 1. + costheta * (-expfBk * fB * fC) * fR +
            3. * exp(fD * (-((fA - expfBk * fC) * fR) + k)) * fD * fK *
            (1. + expfBk * fB * fC * fR) * fW;


    STATE dresktheta = -fR * (fA - fC * expfBk) * sintheta;
    dresl[0] = dresktheta;
    dresl[1] = 0;
    dresl[2] = dreskk;
}

void TPZSandlerDimaggioPV::FromHWCylToPrincipal(const TPZVec<STATE> &HWCylCoords, TPZVec<STATE> &HWCart) {
    const STATE M_SQRT1_3(1. / sqrt(3.));
    const STATE M_SQRT2_3(sqrt(2. / 3.));
    const STATE xi(HWCylCoords[0]);
    const STATE rho(HWCylCoords[1]);
    const STATE beta(HWCylCoords[2]);
    HWCart[0] = M_SQRT1_3 * xi + M_SQRT2_3 * rho * cos(beta);
    HWCart[1] = M_SQRT1_3 * xi + M_SQRT2_3 * rho * cos(beta - (2. * M_PI / 3.));
    HWCart[2] = M_SQRT1_3 * xi + M_SQRT2_3 * rho * cos(beta + (2. * M_PI / 3.));
}

void TPZSandlerDimaggioPV::FromHWCylToHWCart(const TPZVec<STATE> &HWCylCoords, TPZVec<STATE> &cart) {
    cart[0] = HWCylCoords[0];
    cart[1] = HWCylCoords[1] * cos(HWCylCoords[2]);
    cart[2] = HWCylCoords[1] * sin(HWCylCoords[2]);
}

void TPZSandlerDimaggioPV::FromPrincipalToHWCart(const TPZVec<STATE> &PrincipalCoords, TPZVec<STATE> &HWCart) {
    TPZFNMatrix<9, STATE> Rot(3, 3, 0.), temp(3, 1, 0.), cart(3, 1, 0.);
    HWCart.Resize(3, 0.);
    temp(0, 0) = PrincipalCoords[0];
    temp(1, 0) = PrincipalCoords[1];
    temp(2, 0) = PrincipalCoords[2];
    GetRotMatrix(Rot);
    Rot.Multiply(temp, cart);
    HWCart[0] = cart(0, 0);
    HWCart[1] = cart(1, 0);
    HWCart[2] = cart(2, 0);
}

void TPZSandlerDimaggioPV::FromPrincipalToHWCyl(const TPZVec<STATE> &PrincipalCoords, TPZVec<STATE> &HWCyl) {
    TPZFNMatrix<9, STATE> Rot(3, 3, 0.), temp(3, 1, 0.), cart(3, 1, 0.);
    temp(0, 0) = PrincipalCoords[0];
    temp(1, 0) = PrincipalCoords[1];
    temp(2, 0) = PrincipalCoords[2];
    GetRotMatrix(Rot);
    Rot.Multiply(temp, cart);
    HWCyl[0] = cart(0, 0);
    HWCyl[1] = sqrt(cart(1, 0) * cart(1, 0) + cart(2, 0) * cart(2, 0));
    HWCyl[2] = atan2(cart(2, 0), cart(1, 0));
    //    HWCyl[2]=atan(cart(2,0)/cart(1,0));
}

void TPZSandlerDimaggioPV::F1Cyl(STATE xi, STATE beta, TPZVec<STATE> &f1cyl) const {
    const STATE sqrt2 = M_SQRT2;
    const STATE sqrt3 = sqrt(3);
    const STATE I1 = xi*sqrt3;
    STATE sqrtj2 = F(I1);
    STATE rho = sqrt2*sqrtj2;
    f1cyl[0] = xi;
    f1cyl[1] = rho;
    f1cyl[2] = beta;
}

void TPZSandlerDimaggioPV::SurfaceParamF1(TPZVec<STATE> &sigproj, STATE &xi, STATE &beta) const {
    TPZManVector<STATE> sigHWCyl(3);
    FromPrincipalToHWCyl(sigproj, sigHWCyl);
    xi = sigHWCyl[0];
    beta = sigHWCyl[2];
#ifdef PZDEBUG
    STATE dist = DistF1(sigproj, xi, beta);
    if (fabs(dist) > ftol) {
        DebugStop();
    }
#endif
}

void TPZSandlerDimaggioPV::F2Cyl(const STATE theta, const STATE beta, const STATE k, TPZVec<STATE> &f2cyl) const {
    STATE sqrt2 = sqrt(2);
    STATE sqrt3 = sqrt(3);
    STATE Fk = F(k);
    STATE var = fR * Fk * cos(theta);
    STATE I1 = k + var;
    STATE sqrtj2 = Fk * sin(theta);
    STATE rho = sqrt2*sqrtj2;
    STATE xi = I1 / sqrt3;
    f2cyl[0] = xi;
    f2cyl[1] = rho;
    f2cyl[2] = beta;
}

void TPZSandlerDimaggioPV::SurfaceParamF2(const TPZVec<STATE> &sigproj, const STATE k, STATE &theta, STATE &beta) const {
    TPZManVector<STATE> sigHWCyl(3);
    FromPrincipalToHWCyl(sigproj, sigHWCyl);
    STATE I1 = sigHWCyl[0] * sqrt(3.);
    beta = sigHWCyl[2];
    STATE Fk = F(k);
    STATE costheta = (I1 - k) / (fR * Fk);
    STATE sqrtj2 = sigHWCyl[1] / sqrt(2.);
    STATE sintheta = sqrtj2 / Fk;
    theta = atan2(sintheta, costheta);
#ifdef PZDEBUG
    STATE err = 1. - sintheta * sintheta - costheta*costheta;
    STATE dist = DistF2(sigproj, theta, beta, k);
    if (fabs(dist) > ftol || err > ftol) {
        DebugStop();
    }
#endif

}

void TPZSandlerDimaggioPV::GetRotMatrix(TPZFMatrix<STATE> &Rot) {
    Rot.Resize(3, 3);
    Rot(0, 0) = 1. / sqrt(3.);
    Rot(0, 1) = 1. / sqrt(3.);
    Rot(0, 2) = 1. / sqrt(3.);
    Rot(1, 0) = sqrt(2. / 3.);
    Rot(1, 1) = -1. / sqrt(6.);
    Rot(1, 2) = -1. / sqrt(6.);
    Rot(2, 0) = 0;
    Rot(2, 1) = 1. / sqrt(2.);
    Rot(2, 2) = -1. / sqrt(2.);
}

STATE TPZSandlerDimaggioPV::DistF1(const TPZVec<STATE> &pt, const STATE xi, const STATE beta) const {
    //    TPZFNMatrix<9, STATE> Rot(3, 3);
    //    GetRotMatrix(Rot);
    TPZManVector<STATE, 3> cyl(3);
    F1Cyl(xi, beta, cyl);
    TPZManVector<STATE, 3> cart(3);
    FromHWCylToHWCart(cyl, cart);
    TPZManVector<STATE, 3> carttrial(3);
    FromPrincipalToHWCart(pt, carttrial);
    return ((1. / (3. * fK))*(carttrial[0] - cart[0])*(carttrial[0] - cart[0]))
            +(1. / (2. * fG))*((carttrial[1] - cart[1])*(carttrial[1] - cart[1])+(carttrial[2] - cart[2])*(carttrial[2] - cart[2]));

}

STATE TPZSandlerDimaggioPV::DistF2(const TPZVec<STATE> &pt, const STATE theta, const STATE beta, const STATE k) const {
    //    TPZFNMatrix<9, STATE> Rot(3, 3);
    //    GetRotMatrix(Rot);
    TPZManVector<STATE, 3> cyl(3);
    F2Cyl(theta, beta, k, cyl);
    TPZManVector<STATE, 3> cart(3);
    FromHWCylToHWCart(cyl, cart);
    TPZManVector<STATE, 3> carttrial(3);
    FromPrincipalToHWCart(pt, carttrial);
    return ((1. / (3. * fK))*(carttrial[0] - cart[0])*(carttrial[0] - cart[0]))
            +(1. / (2. * fG))*((carttrial[1] - cart[1])*(carttrial[1] - cart[1]) + (carttrial[2] - cart[2])*(carttrial[2] - cart[2]));
}

STATE TPZSandlerDimaggioPV::DistF2IJ(const TPZVec<STATE> &sigtrialIJ, STATE theta, STATE k) const {
    STATE I1 = sigtrialIJ[0];
    STATE sqJ2 = sigtrialIJ[1];
    STATE Fk;
    Fk = F(k);
    STATE y = (sqJ2 - Fk * sin(theta));
    STATE x = 1. / (3 * fK)*(I1 - (k + Fk * fR * cos(theta)));
    STATE res = x * x / (9. * fK) + y * y / (fG);
    return res;
}

template<class T>
void TPZSandlerDimaggioPV::FromThetaKToSigIJ(const T &theta, const T &K, TPZVec<T> &sigIJ) const {
    T Fk = F(K);
    sigIJ[0] = K + Fk * fR * cos(theta);
    sigIJ[1] = Fk * sin(theta);
}

/**
 * compute the value of the equation which determines the orthogonality of the projection
 */
template<class T>
void TPZSandlerDimaggioPV::DDistF2IJ(TPZVec<T> &sigtrialIJ, T theta, T L, STATE LPrev, TPZVec<T> &ddistf2) const {
    T I1 = sigtrialIJ[0];
    T sqJ2 = sigtrialIJ[1];
    T Fk;
    Fk = F(L);
    T y = (sqJ2 - Fk * sin(theta));
    T x = (I1 - (L + Fk * fR * cos(theta)));
    ddistf2[0] = T(2.) * x * Fk * fR * sin(theta) / T(9. * fK) - T(2.) * y * Fk * cos(theta);
    ddistf2[1] = ResLF2IJ(sigtrialIJ, theta, L, LPrev);
#ifdef LOG4CXX
    if (logger->isDebugEnabled()) {
        std::stringstream sout;
        sout << "x = " << x << " y = " << y << " theta = " << theta << " res = " << ddistf2[0];
        LOGPZ_DEBUG(logger, sout.str())
    }
#endif
}

void TPZSandlerDimaggioPV::DDistFunc1(const TPZVec<STATE> &pt, STATE xi, STATE beta, TPZFMatrix<STATE> &ddistf1) const {
    STATE sig1, sig2, sig3, DFf, Ff, I1, sb, cb, Sqrt2, Sqrt3;
    TPZVec<STATE> ptcart(3);
    sb = sin(beta);
    cb = cos(beta);
    FromPrincipalToHWCart(pt, ptcart);
    sig1 = ptcart[0];
    sig2 = ptcart[1];
    sig3 = ptcart[2];
    I1 = xi * sqrt(3);
    Ff = F(I1);
    DFf = -(exp(fB * I1) * fB * fC);
    Sqrt2 = M_SQRT2;
    Sqrt3 = sqrt(3.);

    ddistf1.Resize(2, 1);
    ddistf1(0, 0) = (6 * Sqrt3 * DFf * Ff * fK - 3 * Sqrt3 * Sqrt2 * DFf * fK * (cb * sig2 + sb * sig3) + 2 * fG * (-sig1 + xi)) / (3. * fG * fK);
    ddistf1(1, 0) = -((Ff * Sqrt2 * (-sb * sig2 + cb * sig3)) / fG);
}

void TPZSandlerDimaggioPV::D2DistFunc1(const TPZVec<STATE> &pt, STATE xi, STATE beta, TPZFMatrix<STATE> &tangentf1) const {
    STATE sig2, sig3, DFf, Ff, I1, sb, cb, D2Ff, Sqrt2, Sqrt3;
    TPZVec<STATE> ptcart(3);
    sb = sin(beta);
    cb = cos(beta);
    STATE sin3b = sin(3 * beta);
    STATE cos3b = cos(3 * beta);
    FromPrincipalToHWCart(pt, ptcart);
    //    STATE sig1 = ptcart[0];
    sig2 = ptcart[1];
    sig3 = ptcart[2];
    I1 = xi * sqrt(3);
    Ff = F(I1);
    DFf = -(exp(fB * I1) * fB * fC);
    D2Ff = -(exp(fB * I1) * pow(fB, 2.) * fC);
    Sqrt2 = sqrt(2);
    Sqrt3 = sqrt(3);

    tangentf1.Resize(2, 2);


    tangentf1(0, 0) = (18 * (DFf * DFf + D2Ff * Ff) * fK + 2 * fG -
            9 * D2Ff * fK * (cb * sig2 + sb * sig3) * Sqrt2) / (3. * fG * fK);

    tangentf1(0, 1) = -((Sqrt3 * Sqrt2 * DFf * (-sb * sig2 + cb * sig3)) / fG);

    tangentf1(1, 1) = -((Ff * (- (cb * sig2 + sb * sig3) * Sqrt2)) / fG);

    tangentf1(1, 0) = tangentf1(0, 1);
}

// derivative of the distance function with respect to theta beta k (=L) respectively

template<class T>
void TPZSandlerDimaggioPV::DDistFunc2(const TPZVec<T> &pt, T theta, T beta, T k, T kprev, TPZVec<T> &ddistf2) const {
    T sig1, sig2, sig3, sb, cb, Sqrt2, Sqrt3, FfAlpha, c2t, st, ct, DFAlpha, expBC, s2t;
    TPZVec<T> ptcart(3);
    sb = sin(beta);
    cb = cos(beta);
    st = sin(theta);
    ct = cos(theta);
    c2t = cos(2 * theta);
    s2t = sin(2 * theta);
    T sin3b = sin(3 * beta);
    T cos3b = cos(3 * beta);
    FromPrincipalToHWCart(pt, ptcart);
    sig1 = ptcart[0];
    sig2 = ptcart[1];
    sig3 = ptcart[2];
    FfAlpha = F(k);
    DFAlpha = -exp(fB * k) * fB * fC;
    Sqrt2 = M_SQRT2;
    Sqrt3 = sqrt(3);
    expBC = exp(fB * k) * fB * fC;
    ddistf2.Resize(3, 1);
    ddistf2[0] = (FfAlpha * ((-9 * ct * fK * (cb * sig2 + sb * sig3) * Sqrt2 + 2 * fG * fR * (-k + Sqrt3 * sig1) * st) + FfAlpha * (9 * fK - fG * fR * fR) * s2t)) / (9. * fG * fK);
    ddistf2[1] = (FfAlpha * st * (-((-(sb * sig2) + cb * sig3) * Sqrt2))) / fG;
    ddistf2[2] = ResLF2(pt, theta, beta, k, kprev);
}

void TPZSandlerDimaggioPV::D2DistFunc2(const TPZVec<STATE> &pt, STATE theta, STATE beta, STATE k, TPZFMatrix<STATE> &tangentf2)const {
    STATE sig1, sig2, sig3, sb, cb, Sqrt2, Sqrt3, FfAlpha, c2t, st, ct, DFAlpha, expBC;
    TPZVec<STATE> ptcart(3);
    sb = sin(beta);
    cb = cos(beta);
    st = sin(theta);
    ct = cos(theta);
    c2t = cos(2 * theta);
    FromPrincipalToHWCart(pt, ptcart);
    sig1 = ptcart[0];
    sig2 = ptcart[1];
    sig3 = ptcart[2];
    FfAlpha = F(k);
    DFAlpha = -exp(fB * k) * fB * fC;
    //    STATE D2Gamma2=D2Gamma*D2Gamma;
    Sqrt2 = sqrt(2);
    Sqrt3 = sqrt(3);
    expBC = exp(fB * k) * fB * fC;
    tangentf2.Resize(3, 3);

    tangentf2(0, 0) = (2 * c2t * FfAlpha * FfAlpha * (9 * fK - fG * fR * fR)) / (9. * fG * fK) +
            (FfAlpha * (2 * ct * fG * fR * (-k + Sqrt3 * sig1) + 9 * fK * (cb * sig2 + sb * sig3) * Sqrt2 * st)) / (9. * fG * fK);

    tangentf2(0, 1) = (ct * FfAlpha * (-((-(sb * sig2) + cb * sig3) * Sqrt2))) / fG;
    tangentf2(0, 2) = (-2 * FfAlpha * (-18 * ct * DFAlpha * fK + fG * fR * (1 + 2 * ct * DFAlpha * fR)) * st) /
            (9. * fG * fK) + (DFAlpha * (-9 * ct * fK * (cb * sig2 + sb * sig3) * Sqrt2 + 2 * fG * fR * (-k + Sqrt3 * sig1) * st)) / (9. * fG * fK);

    tangentf2(1, 0) = tangentf2(0, 1);
    tangentf2(1, 1) = (FfAlpha * st * ((cb * sig2 + sb * sig3) * Sqrt2)) / fG;
    tangentf2(1, 2) = -((DFAlpha * Sqrt2 * st * ((-(sb * sig2) + cb * sig3))) /
            (fG));
    tangentf2(2, 0) = -(FfAlpha * fR * st);
    tangentf2(2, 1) = 0;
    tangentf2(2, 2) = 1 - ct * expBC * fR + 3 * fD * exp(fD * (-(FfAlpha * fR) + k)) * fK * (1 + expBC * fR) * fW;
}

void TPZSandlerDimaggioPV::YieldFunction(const TPZVec<STATE> &sigma, STATE kprev, TPZVec<STATE> &yield) const {
    yield.resize(2);
    STATE I1, J2, temp1, temp3, f2, sqrtj2, f1, beta;
    TPZManVector<STATE, 3> cylstress(3);
    FromPrincipalToHWCyl(sigma, cylstress);
    beta = cylstress[2];
    TPZTensor<STATE> sigten;
    sigten.XX() = sigma[0];
    sigten.YY() = sigma[1];
    sigten.ZZ() = sigma[2];
    I1 = sigten.I1();
    J2 = sigten.J2();
    //    STATE JJ3 = sigten.J3();
    if (J2 < 1.e-6) {
        J2 = 1.e-6;
    }
    sqrtj2 = sqrt(J2);

    temp1 = (-I1 + kprev) / (-fR * F(kprev));
    temp3 = sqrtj2 / F(kprev);

    f1 = sqrtj2 - F(I1);
    f2 = temp1 * temp1 + temp3 * temp3 - 1;

    yield[0] = f1;
    yield[1] = f2;
}

void TPZSandlerDimaggioPV::Phi(TPZVec<REAL> sigma, STATE alpha, TPZVec<STATE> &phi)const {

    //    TPZTensor<REAL>::TPZDecomposed DecompSig;
    //    TPZTensor<STATE> sig;
    //    TPZElasticResponse ER;
    //    ER.Compute(eps,sig);
    //    sig.EigenSystem(DecompSig);
    YieldFunction(sigma, alpha, phi);
}

void TPZSandlerDimaggioPV::ProjectF1(const TPZVec<STATE> &sigmatrial, STATE kprev, TPZVec<STATE> &sigproj, STATE &kproj) const {
#ifdef LOG4CXX
    if (loggerConvTest->isDebugEnabled()) {
        std::stringstream outfile;
        outfile << "\n projection over F1 " << endl;
        LOGPZ_DEBUG(loggerConvTest, outfile.str());
    }
#endif

    STATE xi = fA, resnorm, beta = 0., distxi, distnew;
    distxi = std::numeric_limits<STATE>::max();
    STATE guessxi = fA;
    TPZManVector<STATE> sigstar;
    FromPrincipalToHWCart(sigmatrial, sigstar);
    STATE betaguess = atan(sigstar[2] / sigstar[1]);
    for (STATE xiguess = -2 * guessxi; xiguess <= 2 * guessxi; xiguess += 2 * guessxi / 20.) {
        distnew = DistF1(sigmatrial, xiguess, betaguess);
        if (fabs(distnew) < fabs(distxi)) {
            xi = xiguess;
            beta = betaguess;
            distxi = distnew;
        }
    }

    resnorm = std::numeric_limits<STATE>::max();
    long counter = 1;
    TPZFNMatrix<4, STATE> xn1(2, 1, 0.), xn(2, 1, 0.), jac, invjac, sol(2, 1, 0.), fxn(2, 1, 0.);
    xn(0, 0) = xi;
    xn(1, 0) = beta;
    while (resnorm > ftol && counter < 30) {
        TPZFNMatrix<4, STATE> jac(2, 2);
        D2DistFunc1(sigmatrial, xn(0), xn(1), jac);
        DDistFunc1(sigmatrial, xn(0), xn(1), fxn);
        sol = fxn;
        resnorm = Norm(sol);

#ifdef LOG4CXX
        if (loggerConvTest->isDebugEnabled()) {
            std::stringstream outfile;
            outfile << counter << " " << log(resnorm) << endl;
            LOGPZ_DEBUG(loggerConvTest, outfile.str());
        }
#endif

        jac.Solve_LU(&sol);
        xn1 = xn - sol;
        xn = xn1;
        counter++;
    }

    TPZManVector<STATE, 3> sigprojcyl(3);
    F1Cyl(xn[0], xn[1], sigprojcyl);

    if (sigprojcyl[1] < 0) { // rho < 0 implies that sigma is projected to a point outside the F1 curve
        // Pushing the projected sigma back to the curve.
        // We already know J2 should be zero.
        sigprojcyl[1] = 0;
        // Now we need to find I1 s.t. F1(I1, J2=0) = 0.
        //sigprojcyl[0] = ?;

        // The Lode angle (sigprojcyl[2]) is not used in this case, so we don't need to bother with it.
    }
    FromHWCylToPrincipal(sigprojcyl, sigproj);

    STATE kguess = kprev;
    STATE resl = ResLF1(sigmatrial, sigproj, kguess, kprev);
    while (resl < 0.) {
        kguess += 1.;
        resl = ResLF1(sigmatrial, sigproj, kguess, kprev);
    }

    unsigned int count = 0;
    while (fabs(resl) > ftol && count < 30) {
        STATE dresl = DResLF1(sigmatrial, sigproj, kguess, kprev);
        kguess -= resl / dresl;
        resl = ResLF1(sigmatrial, sigproj, kguess, kprev);
        count++;
    }

#ifdef PZDEBUG
    if (count >= 30) {
        DebugStop();
    }
#endif

    kproj = kguess;
}

void TPZSandlerDimaggioPV::ProjectF2(const TPZVec<STATE> &sigmatrial, STATE kprev, TPZVec<STATE> &sigproj, STATE &kproj) const {
#ifdef LOG4CXX
    if (loggerConvTest->isDebugEnabled()) {
        std::stringstream outfile;
        outfile << "\n projection over F2 " << endl;
        LOGPZ_DEBUG(loggerConvTest, outfile.str());

    }
#endif

    STATE theta = 0., beta = 0., distnew;
    STATE resnorm, disttheta;
    disttheta = 1.e8;
    TPZManVector<STATE, 3> vectempcyl(3);
    FromPrincipalToHWCyl(sigmatrial, vectempcyl);
    STATE betaguess = vectempcyl[2];
    for (STATE thetaguess = M_PI / 2.; thetaguess <= M_PI; thetaguess += M_PI / 20.) {
        distnew = DistF2(sigmatrial, thetaguess, betaguess, kprev);
        if (fabs(distnew) < fabs(disttheta)) {
            theta = thetaguess;
            beta = betaguess;
            disttheta = distnew;
        }
    }

    resnorm = 1;
    int counter = 1;
    TPZFNMatrix<3, STATE> xn1(3, 1, 0.), xn(3, 1, 0.), sol(3, 1, 0.), fxn(3, 1, 0.), diff(3, 1, 0.);
    xn(0, 0) = theta;
    xn(1, 0) = beta;
    xn(2, 0) = kprev;
    while (resnorm > ftol && counter < 30) {

        TPZFNMatrix<9, STATE> jac(3, 3);
        D2DistFunc2(sigmatrial, xn(0), xn(1), xn(2), jac);
        TPZManVector<STATE> fxnvec(3);
        DDistFunc2(sigmatrial, xn(0), xn(1), xn(2), kprev, fxnvec);

        for (int k = 0; k < 3; k++) sol(k, 0) = fxnvec[k];
        resnorm = Norm(sol);
#ifdef LOG4CXX
        if (loggerConvTest->isDebugEnabled()) {
            std::stringstream outfile; //("convergencF1.txt");
            outfile << counter << " " << log(resnorm) << endl;
            //jac.Print(outfile);
            //outfile<< "\n xn " << " "<<fxnvec <<endl;
            //outfile<< "\n res " << " "<<fxnvec <<endl;
            LOGPZ_DEBUG(loggerConvTest, outfile.str());
        }
#endif
        jac.Solve_LU(&sol);
        xn1 = xn - sol;
        diff = xn1 - xn;
        //resnorm=Norm(diff);


        xn = xn1;
        counter++;

    }

    STATE thetasol, betasol, ksol;

    thetasol = xn1(0);
    betasol = xn1(1);
    ksol = xn1(2);
    kproj = ksol;

    TPZManVector<STATE, 3> f2cyl(3);
    F2Cyl(thetasol, betasol, ksol, f2cyl);
    FromHWCylToPrincipal(f2cyl, sigproj);
}

void TPZSandlerDimaggioPV::ProjectRing(const TPZVec<STATE> &sigmatrial, STATE kprev, TPZVec<STATE> &sigproj, STATE &kproj) const {
    STATE beta = 0., distnew;
    STATE resnorm, disttheta;
    disttheta = 1.e8;

    for (STATE betaguess = 0; betaguess <= 2 * M_PI; betaguess += M_PI / 20.) {
        distnew = DistF2(sigmatrial, M_PI / 2, betaguess, kprev);
        if (fabs(distnew) < fabs(disttheta)) {
            //            STATE theta = M_PI/2;
            beta = betaguess;
            disttheta = distnew;
        }
    }
    resnorm = 1;
    long counter = 1;
    TPZFMatrix<STATE> xn1(3, 1, 0.), xn(3, 1, 0.), fxn(3, 1, 0.), diff(3, 1, 0.);
    TPZFNMatrix<3, STATE> sol(3, 1, 0.);
    xn(0, 0) = M_PI / 2;
    xn(1, 0) = beta;
    xn(2, 0) = kprev;
    while (resnorm > ftol && counter < 30) {
        TPZFNMatrix<9, STATE> jac(3, 3);
        D2DistFunc2(sigmatrial, xn[0], xn[1], xn[2], jac);
        TPZManVector<STATE> fxnvec(3);
        DDistFunc2(sigmatrial, xn(0), xn(1), xn(2), kprev, fxnvec);
        for (int k = 0; k < 3; k++) fxn(k, 0) = fxnvec[k];

        for (int i = 0; i < 3; i++) {
            jac(i, 0) = 0.;
            jac(0, i) = 0.;
        }
        jac(0, 0) = 1.;
        fxn(0, 0) = 0.;
        sol = fxn;
        resnorm = Norm(sol);
        jac.Solve_LU(&sol);

        xn1(0, 0) = xn(0, 0);
        xn1(1, 0) = xn(1, 0) - sol(1, 0);
        xn1(2, 0) = xn(2, 0) - sol(2, 0);

        //diff=xn1-xn;
        //resnorm=Norm(diff);
        xn = xn1;
        counter++;

    }
    //    cout<< "\n resnorm = "<<resnorm <<endl;
    //    cout<< "\n counter = "<<xn1 <<endl;
    //    cout<< "\n k = "<<xn1[2] <<endl;
    STATE thetasol, betasol, ksol;

    thetasol = xn1[0];
    betasol = xn1[1];
    ksol = xn1[2];

    TPZManVector<STATE, 3> f2cyl(3);
    F2Cyl(thetasol, betasol, ksol, f2cyl);
    FromHWCylToPrincipal(f2cyl, sigproj);

    kproj = ksol;

}

void TPZSandlerDimaggioPV::ProjectBetaConstF2(const TPZVec<STATE> &sigmatrial, STATE kprev, TPZVec<STATE> &sigproj, STATE &kproj) const {
    //#ifdef LOG4CXX
    //    {
    //        std::stringstream outfile;
    //        outfile << "\n projection over F2 " <<endl;
    //        LOGPZ_DEBUG(logger,outfile.str());
    //
    //    }
    //#endif

    STATE theta = 0., beta = 0., distnew;
    STATE resnorm, disttheta;
    disttheta = std::numeric_limits<STATE>::max();
    const STATE betaconst = 0;
    for (STATE thetaguess = 0; thetaguess <= M_PI; thetaguess += M_PI / 20.) {
        distnew = DistF2(sigmatrial, thetaguess, betaconst, kprev);
        if (fabs(distnew) < fabs(disttheta)) {
            theta = thetaguess;
            beta = betaconst;
            disttheta = distnew;
        }
    }

    resnorm = 1;
    int counter = 1;
    TPZFNMatrix<3, STATE> xn1(3, 1, 0.), xn(3, 1, 0.), sol(3, 1, 0.), fxn(3, 1, 0.);
    xn(0, 0) = theta;
    xn(1, 0) = beta;
    xn(2, 0) = kprev;
    while (resnorm > ftol && counter < 30) {
        TPZFNMatrix<9, STATE> jac(3, 3);
        D2DistFunc2(sigmatrial, xn(0), xn(1), xn(2), jac);
        TPZManVector<STATE> fxnvec(3);
        DDistFunc2(sigmatrial, xn(0), xn(1), xn(2), kprev, fxnvec);
        for (int k = 0; k < 3; k++) fxn(k, 0) = fxnvec[k];
        for (int i = 0; i < 3; i++) {
            jac(i, 1) = 0.;
            jac(1, i) = 0.;
        }
        jac(1, 1) = 1.;
        fxn(1, 0) = 0.;
        sol = fxn;
        resnorm = Norm(sol);
        jac.Solve_LU(&sol);

        xn1(0, 0) = xn(0, 0) - sol(0, 0);
        xn1(1, 0) = xn(1, 0);
        xn1(2, 0) = xn(2, 0) - sol(2, 0);

        //        diff=xn1-xn;
        //        resnorm=Norm(diff);
        xn = xn1;
        counter++;

    }
    //if(counter == 30) cout << "resnorm = " << resnorm << std::endl;
    STATE thetasol, betasol, ksol;

    thetasol = xn1(0);
    betasol = xn1(1);
    ksol = xn1(2);
    kproj = ksol;

    TPZManVector<STATE, 3> f2cyl(3);
    F2Cyl(thetasol, betasol, ksol, f2cyl);
    FromHWCylToPrincipal(f2cyl, sigproj);
}

void TPZSandlerDimaggioPV::ComputeI1(TPZVec<STATE> stress, STATE &I1)const {
    STATE sig1, sig2, sig3;
    sig1 = stress[0];
    sig2 = stress[1];
    sig3 = stress[2];
    I1 = sig1 + sig2 + sig3;

}

void TPZSandlerDimaggioPV::ComputeJ2(TPZVec<STATE> stress, STATE &J2)const {
    STATE sig1, sig2, sig3;
    sig1 = stress[0];
    sig2 = stress[1];
    sig3 = stress[2];
    J2 = (2. * sig1 * sig2 + pow(sig1 + (-sig1 - sig2 - sig3) / 3., 2.) +
            pow(sig2 + (-sig1 - sig2 - sig3) / 3., 2.) + 2 * sig1 * sig3 + 2. * sig2 * sig3 +
            pow((-sig1 - sig2 - sig3) / 3. + sig3, 2.)) / 2.;
}

void TPZSandlerDimaggioPV::ApplyStrainComputeElasticStress(TPZVec<STATE> &strain, TPZVec<STATE> &stress)const {
    STATE sig1, sig2, sig3, s1, s2, s3;
    sig1 = strain[0];
    sig2 = strain[1];
    sig3 = strain[2];

    // Deviatoric part of the elastic strain tensor
    s1 = sig1 - (1. / 3.)*(sig1 + sig2 + sig3);
    s2 = sig2 - (1. / 3.)*(sig1 + sig2 + sig3);
    s3 = sig3 - (1. / 3.)*(sig1 + sig2 + sig3);

    stress[0] = s1 * (2 * fG) + fK * (sig1 + sig2 + sig3);
    stress[1] = s2 * (2 * fG) + fK * (sig1 + sig2 + sig3);
    stress[2] = s3 * (2 * fG) + fK * (sig1 + sig2 + sig3);
}

void TPZSandlerDimaggioPV::ApplyStressComputeElasticStrain(TPZVec<STATE> &stress, TPZVec<STATE> &strain)const {
    STATE sig1, sig2, sig3, s1, s2, s3;
    sig1 = stress[0];
    sig2 = stress[1];
    sig3 = stress[2];

    s1 = sig1 - (1. / 3.)*(sig1 + sig2 + sig3);
    s2 = sig2 - (1. / 3.)*(sig1 + sig2 + sig3);
    s3 = sig3 - (1. / 3.)*(sig1 + sig2 + sig3);

    strain[0] = s1 / (2. * fG)+(sig1 + sig2 + sig3) / (9. * fK);
    strain[1] = s2 / (2. * fG)+(sig1 + sig2 + sig3) / (9. * fK);
    strain[2] = s3 / (2. * fG)+(sig1 + sig2 + sig3) / (9. * fK);

}

/**
 * Imposes the specified strain tensor and returns the correspondent stress state.
 *
 * @param[in] epsTotal Imposed total strain tensor
 * @param[out] sigma Resultant stress
 */
void TPZSandlerDimaggioPV::ApplyStrainComputeSigma(TPZVec<STATE> &epst, TPZVec<STATE> &epsp, STATE & kprev, TPZVec<STATE> &epspnext, TPZVec<STATE> &stressnext, STATE & knext) const {

    STATE I1tr, I1proj;

    TPZManVector<STATE, 3> stresstrial(3), yield(2), deltastress(3), delepsp(3), epsT(epst);
    epsT[0] -= epsp[0];
    epsT[1] -= epsp[1];
    epsT[2] -= epsp[2];
    ApplyStrainComputeElasticStress(stresstrial, epsT);
    YieldFunction(stresstrial, kprev, yield);
    ComputeI1(stresstrial, I1tr);

    if ((yield[1] <= 0 && I1tr <= kprev) || (yield[0] <= 0 && I1tr > kprev)) {

        epspnext = epsp;
        knext = kprev;
        stressnext = stresstrial;
        //cout<<"\n elastic "<<endl;

    } else {
        //cout<<"\n plastic "<<endl;
        if (yield[1] > 0 && I1tr < kprev) {
            //cout<<"\n F2 "<<endl;
            ProjectF2(stresstrial, kprev, stressnext, knext);
        } else {
            //cout<<"\n F1 "<<endl;
            ProjectF1(stresstrial, kprev, stressnext, knext);
            ComputeI1(stressnext, I1proj);
            if (I1proj < knext) {
                //cout<<"\n Ring "<<endl;
                ProjectRing(stresstrial, kprev, stressnext, knext);
            }
        }

        for (int i = 0; i < 3; i++) {
            deltastress[i] = stresstrial[i] - stressnext[i];
        }

        ApplyStressComputeElasticStrain(deltastress, delepsp);

        for (int i = 0; i < 3; i++) {
            epspnext[i] = epsp[i] + delepsp[i];
        }
    }
}

void TPZSandlerDimaggioPV::ProjectSigma(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &sigproj, STATE &kproj) const {
    STATE I1;
    //Firstk(epspv,k0);
    TPZManVector<STATE, 2> yield(2);
    I1 = sigtrial[0] + sigtrial[1] + sigtrial[2];

    YieldFunction(sigtrial, kprev, yield);

    if (I1 < kprev) {
        if (yield[1] > 0.) {
            ProjectF2(sigtrial, kprev, sigproj, kproj);
#ifdef PZDEBUG
            {
                TPZManVector<STATE> cyltr(3), cylproj(3);
                FromPrincipalToHWCyl(sigtrial, cyltr);
                FromPrincipalToHWCyl(sigproj, cylproj);
                //std::cout << "cyltr " << cyltr << " cylpr " << cylproj << std::endl;
            }
#endif
        } else {
            sigproj = sigtrial;
            kproj = kprev;
        }
    } else {
        if (yield[0] > 0.) {
            ProjectF1(sigtrial, kprev, sigproj, kproj);
            // this is a wrong condition!!
            I1 = 0.;
            for (int i = 0; i < 3; i++) {
                I1 += sigproj[i];
            }
            if (I1 < kproj) {
                ProjectRing(sigtrial, kprev, sigproj, kproj);
            }

        } else {
            // elastic behaviour
            sigproj = sigtrial;
            kproj = kprev;
        }
    }

}

/*
void TPZSandlerExtended::ProjectSigmaDep(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &sigproj,STATE &kproj, TPZFMatrix<STATE> &GradSigma) const
{
    STATE I1;
    //Firstk(epspv,k0);
    TPZManVector<STATE,2> yield(2);
    I1 = sigtrial[0]+sigtrial[1]+sigtrial[2];
    
    YieldFunction(sigtrial,kprev,yield);
    bool treeEigEqual = false;
    STATE tol=1.e-8;
    if (fabs(sigtrial[0]-sigtrial[1])<tol && fabs(sigtrial[1]-sigtrial[2])<tol) {
        treeEigEqual=true;
    }
    
    
    if (I1<kprev)
    {
        if (yield[1]>0. && treeEigEqual==false)
        {
            ProjectF2(sigtrial,kprev,sigproj,kproj);
            // we can compute the tangent matrix
            TPZFNMatrix<9,STATE> dbetadsigtrial(3,3), jacF2(3,3), DF2cart(3,3);
            STATE theta,beta;
            SurfaceParamF2(sigproj, kproj, theta, beta);
            GradF2SigmaTrial(sigtrial, theta, beta, kproj, kprev, dbetadsigtrial);
            D2DistFunc2(sigtrial, theta, beta, kproj, jacF2);
            jacF2.Solve_LU(&dbetadsigtrial);
            DF2Cart(theta, beta, kproj, DF2cart);
            DF2cart.Multiply(dbetadsigtrial, GradSigma);
            GradSigma *= -1.;
        }
        else if (yield[1]>0. && treeEigEqual==true)
        {
            ProjectBetaConstF2(sigtrial,kprev,sigproj,kproj);
            // we can compute the tangent matrix
            TPZFNMatrix<9,STATE> dbetadsigtrial(3,3), jacF2(3,3), DF2cart(3,3);
            STATE theta,beta;
            SurfaceParamF2(sigproj, kproj, theta, beta);
            beta=0;
            //#ifdef PZDEBUG
            //            if(fabs(sigproj[1]) > tol)
            //            {
            //                DebugStop();
            //            }
            //#endif
            GradF2SigmaTrial(sigtrial, theta, beta, kproj, kprev, dbetadsigtrial);
            for(int i=0; i<3; i++) dbetadsigtrial(1,i) = 0.;
            D2DistFunc2(sigtrial, theta, beta, kproj, jacF2);
            for (int i=0; i<3; i++) {
                jacF2(i,1) = 0.;
                jacF2(1,i) = 0.;
            }
            jacF2(1,1) = 1.;
            jacF2.Solve_LU(&dbetadsigtrial);
            DF2Cart(theta, beta, kproj, DF2cart);
            for(int i=0; i<3; i++) DF2cart(i,1) = 0.;
            DF2cart.Multiply(dbetadsigtrial, GradSigma);
            GradSigma *= -1.;
            
        }
        else
        {
            sigproj = sigtrial;
            kproj = kprev;
            GradSigma.Identity();
        }
    }
    else
    {
        if (yield[0]>0.)
        {
            ProjectF1(sigtrial,kprev,sigproj,kproj);
            
            I1 = 0.;
            for (int i=0; i<3; i++) {
                I1 += sigproj[i];
            }
            if (I1<kproj)
            {
                ProjectRing(sigtrial,kprev,sigproj,kproj);
                
                // we can compute the tangent matrix
                TPZFNMatrix<9,STATE> dbetadsigtrial(3,3), jacF2(3,3), DF2cart(3,3);
                STATE theta,beta;
                SurfaceParamF2(sigproj, kproj, theta, beta);
#ifdef PZDEBUG
                if(fabs(theta - M_PI_2) > 1.e-8)
                {
                    DebugStop();
                }
#endif
                GradF2SigmaTrial(sigtrial, theta, beta, kproj, kprev, dbetadsigtrial);
                for(int i=0; i<3; i++) dbetadsigtrial(0,i) = 0.;
                D2DistFunc2(sigtrial, theta, beta, kproj, jacF2);
                for (int i=0; i<3; i++) {
                    jacF2(i,0) = 0.;
                    jacF2(0,1) = 0.;
                }
                jacF2(0,0) = 1.;
                jacF2.Solve_LU(&dbetadsigtrial);
                DF2Cart(theta, beta, kproj, DF2cart);
                for(int i=0; i<3; i++) DF2cart(i,0) = 0.;
                DF2cart.Multiply(dbetadsigtrial, GradSigma);
                GradSigma *= -1.;
            }
            else
            {
                // we can compute the tangent matrix
                TPZFNMatrix<9,STATE> dbetadsigtrial(2,3), jacF1(2,2), DF1cart(3,2);
                STATE xi,beta;
                SurfaceParamF1(sigproj, xi, beta);
                GradF1SigmaTrial(sigtrial, xi, beta, dbetadsigtrial);
                D2DistFunc1(sigtrial, xi, beta, jacF1);
                jacF1.Solve_LU(&dbetadsigtrial);
                DF1Cart(xi, beta, DF1cart);
                DF1cart.Multiply(dbetadsigtrial, GradSigma);
                GradSigma *= -1.;
            }
            
        }
        else
        {
            // elastic behaviour
            sigproj = sigtrial;
            kproj=kprev;
            GradSigma.Identity();
        }
    }
    
}

 */

void TPZSandlerDimaggioPV::ProjectSigmaDep(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &sigproj, STATE &kproj, TPZFMatrix<STATE> &GradSigma) const {
    STATE I1;
    //Firstk(epspv,k0);
    TPZManVector<STATE, 2> yield(2);
    I1 = sigtrial[0] + sigtrial[1] + sigtrial[2];

    YieldFunction(sigtrial, kprev, yield);
    STATE tol = 1.e-8;
    bool threeEigEqual = (fabs(sigtrial[0] - sigtrial[1]) < tol) && (fabs(sigtrial[1] - sigtrial[2]) < tol);

    if (I1 < kprev) { // f2: strain-hardening cap
        if (yield[1] > 0.) {
            if (threeEigEqual) { // Equal eigenvalues
#ifdef LOG4CXX
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    sout << "Projecting on F2, equal eigenvalues " << sigtrial;
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
                ProjectBetaConstF2(sigtrial, kprev, sigproj, kproj);
                // we can compute the tangent matrix
                STATE theta, beta;
                // compute theta beta as a function of sigproj, kproj
                SurfaceParamF2(sigproj, kproj, theta, beta);
                // theta should be Pi
                // for hydrostatic stress beta doesn't mean anything
#ifdef LOG4CXX
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    sout << "Surface parameters for sigproj = " << sigproj << " kproj " << kproj << " theta " << theta;
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
                TPZManVector<STATE, 2> sigtrialIJ(2, 0.), sigprojIJ(2), ddistf2(2);
                sigtrialIJ[0] = sigtrial[0] + sigtrial[1] + sigtrial[2];
                DDistF2IJ(sigtrialIJ, theta, kproj, kprev, ddistf2);
#ifdef LOG4CXX
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    sout << "Derivative of the distance function (should be zero) = " << ddistf2;
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
                TPZFNMatrix<4, STATE> JacThetaK(2, 2), JacSigtrIJ(2, 2), JacSigprojThetaK(2, 2);
                {
                    TPZManVector<TFad<2, STATE>, 2> sigtrialIJFAD(2), ddistf2fad(2);
                    TFad<2, STATE> thetafad(theta, 0), kprojfad(kproj, 1);
                    sigtrialIJFAD[0].val() = sigtrialIJ[0];
                    sigtrialIJFAD[1].val() = sigtrialIJ[1];
                    DDistF2IJ(sigtrialIJFAD, thetafad, kprojfad, kprev, ddistf2fad);
                    JacThetaK(0, 0) = ddistf2fad[0].d(0);
                    JacThetaK(0, 1) = ddistf2fad[0].d(1);
                    JacThetaK(1, 0) = ddistf2fad[1].d(0);
                    JacThetaK(1, 1) = ddistf2fad[1].d(1);
                }
                {
                    TPZManVector<TFad<2, STATE>, 2> sigtrialIJFAD(2), ddistf2fad(2);
                    TFad<2, STATE> thetafad(theta), kprojfad(kproj);
                    sigtrialIJFAD[0].val() = sigtrialIJ[0];
                    sigtrialIJFAD[0].fastAccessDx(0) = 1.;
                    sigtrialIJFAD[1].val() = sigtrialIJ[1];
                    sigtrialIJFAD[1].fastAccessDx(1) = 1.;
                    DDistF2IJ(sigtrialIJFAD, thetafad, kprojfad, kprev, ddistf2fad);
                    JacSigtrIJ(0, 0) = ddistf2fad[0].d(0);
                    JacSigtrIJ(0, 1) = ddistf2fad[0].d(1);
                    JacSigtrIJ(1, 0) = ddistf2fad[1].d(0);
                    JacSigtrIJ(1, 1) = ddistf2fad[1].d(1);
                }
                FromThetaKToSigIJ(theta, kproj, sigprojIJ);
                {
                    TPZManVector<TFad<2, STATE>, 2> sigprojIJFAD(2);
                    TFad<2, STATE> thetafad(theta, 0), kprojfad(kproj, 1);
                    FromThetaKToSigIJ(thetafad, kprojfad, sigprojIJFAD);
                    JacSigprojThetaK(0, 0) = sigprojIJFAD[0].d(0);
                    JacSigprojThetaK(0, 1) = sigprojIJFAD[0].d(1);
                    JacSigprojThetaK(1, 0) = sigprojIJFAD[1].d(0);
                    JacSigprojThetaK(1, 1) = sigprojIJFAD[1].d(1);
                }
#ifdef LOG4CXX
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    sout << "Derivative of the distanceIJ " << ddistf2 << std::endl;
                    JacThetaK.Print("Derivative of distanceIJ with respect to theta, K", sout);
                    JacSigtrIJ.Print("Derivative of distanceIJ with respect to sigtialIJ", sout);
                    sout << "SigmaProjected IJ " << sigprojIJ << std::endl;
                    JacSigprojThetaK.Print("Derivative of sigproj with respect to theta K", sout);
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
                std::list<long> singular;
                JacThetaK.Solve_LU(&JacSigtrIJ, singular);
#ifdef LOG4CXX
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    sout << "Negative of derivative of Theta,K with respect to sigtrIJ" << std::endl;
                    JacSigtrIJ.Print("Derivative = ", sout);
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
                TPZFNMatrix<4, STATE> dsigprojdsigtr(2, 2);
                JacSigprojThetaK.Multiply(JacSigtrIJ, dsigprojdsigtr);
                dsigprojdsigtr *= -1.;
#ifdef LOG4CXX
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    dsigprojdsigtr.Print("Derivative of SigprojIJ with respect to SigtrialIJ", sout);
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        GradSigma(i, j) = dsigprojdsigtr(0, 0) / 3.;
                        if (i == j) {
                            GradSigma(i, j) += dsigprojdsigtr(1, 1)*2. / 3.;
                        } else {
                            GradSigma(i, j) -= dsigprojdsigtr(1, 1) / 3.;
                        }
                    }
                }
            } else { // Distinct eigenvalues
#ifdef LOG4CXX
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    sout << "Projecting on F2, distinct eigenvalues " << sigtrial;
                    LOGPZ_DEBUG(logger, sout.str())
                }
#endif
                ProjectF2(sigtrial, kprev, sigproj, kproj);
                // we can compute the tangent matrix
                TPZFNMatrix<9, STATE> dbetadsigtrial(3, 3), jacF2(3, 3), DF2cart(3, 3);
                STATE theta, beta;
                SurfaceParamF2(sigproj, kproj, theta, beta);
                GradF2SigmaTrial(sigtrial, theta, beta, kproj, kprev, dbetadsigtrial);
                D2DistFunc2(sigtrial, theta, beta, kproj, jacF2);
                jacF2.Solve_LU(&dbetadsigtrial);
                DF2Cart(theta, beta, kproj, DF2cart);
                DF2cart.Multiply(dbetadsigtrial, GradSigma);
                GradSigma *= -1.;
            }
        } else { // yield[1] <= 0: we do not have to project
            sigproj = sigtrial;
            kproj = kprev;
            GradSigma.Identity();
        }
    } else { // f1: modified Drucker-Prager yield function
        if (yield[0] > 0.) { // plastic behavior: we have to project back to the f1 yield function
#ifdef LOG4CXX
            if (logger->isDebugEnabled()) {
                std::stringstream sout;
                sout << "Projecting on F1";
                LOGPZ_DEBUG(logger, sout.str())
            }
#endif
            ProjectF1(sigtrial, kprev, sigproj, kproj);

            I1 = sigproj[0] + sigproj[1] + sigproj[2];
            if (I1 < kproj) {
                ProjectRing(sigtrial, kprev, sigproj, kproj);

                // we can compute the tangent matrix
                TPZFNMatrix<9, STATE> dbetadsigtrial(3, 3), jacF2(3, 3), DF2cart(3, 3);
                STATE theta, beta;
                SurfaceParamF2(sigproj, kproj, theta, beta);
#ifdef PZDEBUG
                if (fabs(theta - M_PI_2) > 1.e-8) {
                    DebugStop();
                }
#endif
                GradF2SigmaTrial(sigtrial, theta, beta, kproj, kprev, dbetadsigtrial);
                for (int i = 0; i < 3; i++) dbetadsigtrial(0, i) = 0.;
                D2DistFunc2(sigtrial, theta, beta, kproj, jacF2);
                for (int i = 0; i < 3; i++) {
                    jacF2(i, 0) = 0.;
                    jacF2(0, 1) = 0.;
                }
                jacF2(0, 0) = 1.;
                jacF2.Solve_LU(&dbetadsigtrial);
                DF2Cart(theta, beta, kproj, DF2cart);
                for (int i = 0; i < 3; i++) DF2cart(i, 0) = 0.;
                DF2cart.Multiply(dbetadsigtrial, GradSigma);
                GradSigma *= -1.;
            } else {
                // we can compute the tangent matrix
                TPZFNMatrix<9, STATE> dbetadsigtrial(2, 3), jacF1(2, 2), DF1cart(3, 2);
                STATE xi, beta;
                SurfaceParamF1(sigproj, xi, beta);
                GradF1SigmaTrial(sigtrial, xi, beta, dbetadsigtrial);
                D2DistFunc1(sigtrial, xi, beta, jacF1);
                jacF1.Solve_LU(&dbetadsigtrial);
                DF1Cart(xi, beta, DF1cart);
                DF1cart.Multiply(dbetadsigtrial, GradSigma);
                GradSigma *= -1.;
            }
        } else { // yield[0] <= 0.: we do not have to project
#ifdef LOG4CXX
            {
                if (logger->isDebugEnabled()) {
                    std::stringstream sout;
                    sout << "Elastic Behaviour";
                    LOGPZ_DEBUG(logger, sout.str())
                }
            }
#endif
            // elastic behaviour
            sigproj = sigtrial;
            kproj = kprev;
            GradSigma.Identity();
        }
    }
}


#define Cos cos
#define Sin sin
#define Sqrt sqrt


/// Compute the derivative of the residual with respect to sigtrial

void TPZSandlerDimaggioPV::GradF1SigmaTrial(const TPZVec<STATE> &sigtrial, STATE xi, STATE beta, TPZFMatrix<STATE> &deriv) const {
    STATE SQR3 = sqrt(3.);
    STATE FFI = fA - fC * exp(fB * SQR3 * xi);
    deriv.Redim(2, 3);
    deriv(0, 0) = (-2 * (fG * 2.0 - 6 * fK * (SQR3 * fA * fB - SQR3 * fB * FFI) * Cos(beta))) / (6. * SQR3 * fG * fK);
    deriv(1, 0) = (4 * FFI * (2.0 * Sin(beta))) / (4.0 * SQR3 * fG);

    deriv(0, 1) = (-2 * (2*SQR3 * fG + 9 * fK * (fA * fB - fB * FFI) * Cos(beta) -
            9 * fK * (SQR3 * fA * fB - fB * SQR3 * FFI) * Sin(beta))) / (18. * fG * fK);
    deriv(1, 1) = (-2 * FFI * (6 * Cos(beta) + 2*SQR3 * Sin(beta))) / (12. * fG);

    deriv(0, 2) = (-2 * (2*SQR3 * fG + 9 * fK * (fA * fB - fB * FFI) * Cos(beta) +
            9 * fK * (SQR3 * fA * fB - fB * (SQR3 * FFI)) * Sin(beta))) / (18. * fG * fK);
    deriv(1, 2) = (-2 * FFI * (- 6 * Cos(beta) + (2*SQR3) * Sin(beta))) / (12. * fG);
}

/// Compute the derivative of the F2 residual with respecto do sigtrial

void TPZSandlerDimaggioPV::GradF2SigmaTrial(const TPZVec<STATE> &sigtrial, STATE theta, STATE beta, STATE k, STATE kprev, TPZFMatrix<STATE> &deriv) const {
    STATE SQR3 = sqrt(3.);
    STATE FFK = fA - fC * exp(fB * k);
    deriv.Redim(3, 3);
    deriv(0, 0) = (-4 * FFK * Cos(beta) * Cos(theta)) / (2*SQR3 * fG) + (2 * FFK * fR * Sin(theta)) / (9. * fK);
    deriv(1, 0) = (4 * FFK * (2 * Sin(beta)) * Sin(theta)) / (4*SQR3 * fG);
    deriv(2, 0) = -1.;

    deriv(0, 1) = (2 * (3 * SQR3 * fK * FFK * Cos(beta) * Cos(theta) - 9 * fK * FFK * Cos(theta) * Sin(beta) + FFK * fG * fR * 2 * Sin(theta))) / (18. * fG * fK);
    deriv(1, 1) = (-2 * FFK * (6* Cos(beta) + 2*SQR3  * Sin(beta)) * Sin(theta)) / (12. * fG);
    deriv(2, 1) = -1.;

    deriv(0, 2) = (2 * (3 * SQR3 * fK * FFK * Cos(beta) * Cos(theta) + 9 * fK * FFK * Cos(theta) * Sin(beta) + FFK * fG * fR * 2 * Sin(theta))) / (18. * fG * fK);
    deriv(1, 2) = (-2 * FFK * (-6 * Cos(beta) + 2*SQR3 * Sin(beta)) * Sin(theta)) / (12. * fG);
    deriv(2, 2) = -1.;

}

void TPZSandlerDimaggioPV::TaylorCheckDistF1(const TPZVec<STATE> &sigmatrial, STATE xi, STATE beta, TPZVec<STATE> &xnorm,
        TPZVec<STATE> &errnorm) const {
    STATE deltaxi = 0.4;
    STATE deltabeta = 0.05;
    STATE dist0 = DistF1(sigmatrial, xi, beta);
    TPZFNMatrix<4, STATE> jac(2, 1);
    DDistFunc1(sigmatrial, xi, beta, jac);
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        STATE diffxi = deltaxi * i / 10.;
        STATE xinext = xi + diffxi;
        STATE diffbeta = deltabeta * i / 10.;
        STATE betanext = beta + diffbeta;
        STATE distnext = DistF1(sigmatrial, xinext, betanext);
        STATE distguess = dist0 + jac(0, 0) * diffxi + jac(1, 0) * diffbeta;
        xnorm[i - 1] = sqrt(diffxi * diffxi + diffbeta * diffbeta);
        errnorm[i - 1] = fabs(distnext - distguess);
    }

}

void TPZSandlerDimaggioPV::TaylorCheckDDistF1(const TPZVec<STATE> &sigmatrial, STATE xi, STATE beta, TPZVec<STATE> &xnorm,
        TPZVec<STATE> &errnorm) const {
    STATE deltaxi = 0.4;
    STATE deltabeta = 0.05;
    TPZFNMatrix<2, STATE> res0(2, 1), resid(2, 1), residguess(2, 1), diff(2, 1);
    TPZFNMatrix<4, STATE> jac(2, 2);
    DDistFunc1(sigmatrial, xi, beta, res0);
    D2DistFunc1(sigmatrial, xi, beta, jac);
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        STATE diffxi = deltaxi * i / 10.;
        STATE xinext = xi + diffxi;
        STATE diffbeta = deltabeta * i / 10.;
        diff(0) = diffxi;
        diff(1) = diffbeta;
        STATE betanext = beta + diffbeta;
        DDistFunc1(sigmatrial, xinext, betanext, resid);
        jac.Multiply(diff, residguess);
        residguess += res0;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

void TPZSandlerDimaggioPV::TaylorCheckDDistF1DSigtrial(const TPZVec<STATE> &sigmatrial, STATE xi, STATE beta, TPZVec<STATE> &xnorm,
        TPZVec<STATE> &errnorm) const {
    TPZManVector<STATE, 3> deltasigma(3, 0.3);
    deltasigma[1] *= -1.;
    deltasigma[2] *= 0.7;

    TPZFNMatrix<2, STATE> res0(2, 1), resid(2, 1), residguess(2, 1), diff(3, 1);
    TPZFNMatrix<6, STATE> jac(2, 3);
    DDistFunc1(sigmatrial, xi, beta, res0);
    GradF1SigmaTrial(sigmatrial, xi, beta, jac);
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        for (int j = 0; j < 3; j++) diff(j) = deltasigma[j] * i / 10.;
        TPZManVector<STATE, 3> sigmanext(3);
        for (int j = 0; j < 3; j++) sigmanext[j] = sigmatrial[j] + diff(j);
        DDistFunc1(sigmanext, xi, beta, resid);
        jac.Multiply(diff, residguess);
        residguess += res0;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

void TPZSandlerDimaggioPV::ConvergenceRate(TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm, TPZVec<STATE> &convergence) {
    convergence.resize(xnorm.size() - 1);
    for (int i = 1; i < xnorm.size(); i++) {
        convergence[i - 1] = log(errnorm[i] / errnorm[i - 1]) / log(xnorm[i] / xnorm[i - 1]);
    }
}

void TPZSandlerDimaggioPV::TaylorCheckDistF2(const TPZVec<STATE> &sigmatrial, STATE theta, STATE beta, STATE k, STATE kprev, TPZVec<STATE> &xnorm,
        TPZVec<STATE> &errnorm) const {
    STATE deltatheta = 0.4;
    STATE deltabeta = 0.05;
    STATE deltak = 0.;
    STATE dist0 = DistF2(sigmatrial, theta, beta, k);
    TPZFNMatrix<4, STATE> jac(3, 1);
    TPZManVector<STATE> fxnvec(3);
    DDistFunc2(sigmatrial, theta, beta, k, kprev, fxnvec);
    for (int kk = 0; kk < 3; kk++) jac(kk, 0) = fxnvec[kk];
    //    DDistFunc2(sigmatrial, theta, beta, k, kprev, jac);
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        STATE difftheta = deltatheta * i / 10.;
        STATE thetanext = theta + difftheta;
        STATE diffbeta = deltabeta * i / 10.;
        STATE betanext = beta + diffbeta;
        STATE diffk = deltak * i / 10.;
        STATE knext = k + deltak;
        STATE distnext = DistF2(sigmatrial, thetanext, betanext, knext);
        STATE distguess = dist0 + jac(0, 0) * difftheta + jac(1, 0) * diffbeta + jac(2, 0) * diffk;
        xnorm[i - 1] = sqrt(difftheta * difftheta + diffbeta * diffbeta + diffk * diffk);
        errnorm[i - 1] = fabs(distnext - distguess);
    }

}

void TPZSandlerDimaggioPV::TaylorCheckDDistF2(const TPZVec<STATE> &sigmatrial, STATE theta, STATE beta, STATE k, STATE kprev, TPZVec<STATE> &xnorm,
        TPZVec<STATE> &errnorm) const {
    STATE deltatheta = 0.4;
    STATE deltabeta = 0.05;
    STATE deltak = 0.5;
    TPZFNMatrix<3, STATE> res0(3, 1), resid(3, 1), residguess(3, 1), diff(3, 1);
    TPZFNMatrix<9, STATE> jac(3, 3);
    TPZManVector<STATE> fxnvec(3);
    DDistFunc2(sigmatrial, theta, beta, k, kprev, fxnvec);
    for (int kk = 0; kk < 3; kk++) res0(kk, 0) = fxnvec[kk];
    //    DDistFunc2(sigmatrial, theta, beta, k, kprev, res0);
    D2DistFunc2(sigmatrial, theta, beta, k, jac);
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        STATE difftheta = deltatheta * i / 10.;
        STATE thetanext = theta + difftheta;
        STATE diffbeta = deltabeta * i / 10.;
        STATE betanext = beta + diffbeta;
        STATE diffk = deltak * i / 10.;
        STATE knext = k + diffk;
        diff(0) = difftheta;
        diff(1) = diffbeta;
        diff(2) = diffk;
        TPZManVector<STATE> fxnvec(3);
        DDistFunc2(sigmatrial, thetanext, betanext, knext, kprev, fxnvec);
        for (int k = 0; k < 3; k++) resid(k, 0) = fxnvec[k];
        //        DDistFunc2(sigmatrial, thetanext, betanext, knext, kprev, resid);
        jac.Multiply(diff, residguess);
        residguess += res0;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

/// teste da derivada D(ResF2)/D(sigtrial)

void TPZSandlerDimaggioPV::TaylorCheckDDistF2DSigtrial(const TPZVec<STATE> &sigmatrial, STATE theta, STATE beta, STATE k, STATE kprev, TPZVec<STATE> &xnorm,
        TPZVec<STATE> &errnorm) const {
    TPZManVector<STATE, 3> deltasigma(3, 0.3);
    deltasigma[1] *= -1.;
    deltasigma[2] *= 0.7;

    TPZFNMatrix<3, STATE> res0(3, 1), resid(3, 1), residguess(3, 1), diff(3, 1);
    TPZFNMatrix<9, STATE> jac(3, 3);
    TPZManVector<STATE> fxnvec(3);
    DDistFunc2(sigmatrial, theta, beta, k, kprev, fxnvec);
    for (int kk = 0; kk < 3; kk++) res0(kk, 0) = fxnvec[kk];
    //    DDistFunc2(sigmatrial, theta, beta, k, kprev, res0);
    GradF2SigmaTrial(sigmatrial, theta, beta, k, kprev, jac);
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        for (int j = 0; j < 3; j++) diff(j) = deltasigma[j] * i / 10.;
        TPZManVector<STATE, 3> sigmanext(3);
        for (int j = 0; j < 3; j++) sigmanext[j] = sigmatrial[j] + diff(j);
        TPZManVector<STATE> fxnvec(3);
        DDistFunc2(sigmanext, theta, beta, k, kprev, fxnvec);
        for (int k = 0; k < 3; k++) resid(k, 0) = fxnvec[k];
        //        DDistFunc2(sigmanext, theta, beta,k,kprev,resid);
        jac.Multiply(diff, residguess);
        residguess += res0;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

#include "pzvec_extras.h"

void TPZSandlerDimaggioPV::CheckCoordinateTransformation(TPZVec<STATE> &cart) {
    TPZManVector<STATE, 3> HWCart(3), HWCyl(3), HWCart2(3), Cart2(3);
    FromPrincipalToHWCart(cart, HWCart);
    FromPrincipalToHWCyl(cart, HWCyl);
    FromHWCylToHWCart(HWCyl, HWCart2);
    FromHWCylToPrincipal(HWCyl, Cart2);
    REAL dist1 = dist(cart, Cart2);
    REAL dist2 = dist(HWCart, HWCart2);
    cout << __FUNCTION__ << " dist1 = " << dist1 << " dist2 = " << dist2 << endl;

}

/// Compute the derivative of the stress (principal s;tresses) as a function of xi and beta

void TPZSandlerDimaggioPV::DF1Cart(STATE xi, STATE beta, TPZFMatrix<STATE> &DF1) const {
    STATE SQR3 = sqrt(3.);
    STATE FFI = fA - fC * exp(fB * SQR3 * xi);
    DF1.Resize(3, 2);
    DF1(0, 0) = 1 / SQR3 + (4 * (-SQR3 * fB * (fA - FFI)) * Cos(beta)) /
            (2*SQR3);
    DF1(1, 0) = 1 / SQR3 + (4 * (-
            SQR3 * fB * (fA - FFI)) * Sin(beta - M_PI / 6.)) / (2*SQR3);
    DF1(2, 0) =
            1 / SQR3 - (4 * (-SQR3 * fB * (fA - FFI)) *
            Sin(beta + M_PI / 6.)) / (2*SQR3);

    DF1(0, 1) = - (4 * FFI * Sin(beta)) / (2*SQR3);
    DF1(1, 1) = (4 * FFI * Cos(beta - M_PI / 6.)) / (2*SQR3);

    DF1(2, 1) = (-4 * FFI * Cos(beta + M_PI / 6.)) / (2*SQR3);
}

/// Compute the derivative of the stress (principal s;tresses) as a function of xi and beta

void TPZSandlerDimaggioPV::DF2Cart(STATE theta, STATE beta, STATE k, TPZFMatrix<STATE> &DF2) const {
    STATE SQR3 = sqrt(3.);
    STATE FFK = fA - fC * exp(fB * k);
    DF2.Resize(3, 3);
    DF2(0, 0) = (4 * FFK * Cos(beta) * Cos(theta)) / (2*SQR3) - (FFK * fR * Sin(theta)) / 3.;
    DF2(1, 0) = (4 * FFK * Cos(theta) * Sin(beta - M_PI / 6.)) / (2*SQR3) - (FFK * fR * Sin(theta)) / 3.;
    DF2(2, 0) = (4 * (-FFK) * Cos(theta) * Sin(beta + M_PI / 6.)) / (2*SQR3) - (FFK * fR * Sin(theta)) / 3.;

    DF2(0, 1) = (-4 * FFK * (2 * Sin(beta)) * Sin(theta)) / (4*SQR3);
    DF2(1, 1) = (4 * FFK * (2 * Cos(beta - M_PI / 6.)) * Sin(theta)) / (4*SQR3);
    DF2(2, 1) = (-4 * FFK * (2 * Cos(beta + M_PI / 6.)) * Sin(theta)) / (4*SQR3);

    DF2(0, 2) = (2 - (fA * fB - fB * FFK)*
            (fR * 2 * Cos(theta) + 4 * SQR3 * Cos(beta) * Sin(theta))) / 6;
    DF2(1, 2) = (2 - (fA * fB - fB * FFK)*
            (fR * 2 * Cos(theta) + 4 * SQR3 * Sin(beta - M_PI / 6.) * Sin(theta))) / 6;
    DF2(2, 2) = (2 - (fA * fB - fB * FFK)*
            (fR * 2 * Cos(theta) - 4 * SQR3 * Sin(beta + M_PI / 6.) * Sin(theta))) / 6;
}

void TPZSandlerDimaggioPV::TaylorCheckDF1Cart(STATE xi, STATE beta, TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm) const {
    STATE deltaxi = 0.4;
    STATE deltabeta = 0.05;
    TPZFNMatrix<3, STATE> res0(3, 1), resid(3, 1), residguess(3, 1), diff(2, 1);
    TPZFNMatrix<9, STATE> jac(3, 2);
    TPZManVector<STATE> sigHWCyl(3), sigCart(3);
    F1Cyl(xi, beta, sigHWCyl);
    FromHWCylToPrincipal(sigHWCyl, sigCart);
    for (int i = 0; i < 3; i++) {
        res0(i) = sigCart[i];
    }
    DF1Cart(xi, beta, jac);
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        STATE diffxi = deltaxi * i / 10.;
        STATE xinext = xi + diffxi;
        STATE diffbeta = deltabeta * i / 10.;
        diff(0) = diffxi;
        diff(1) = diffbeta;
        STATE betanext = beta + diffbeta;
        F1Cyl(xinext, betanext, sigHWCyl);
        FromHWCylToPrincipal(sigHWCyl, sigCart);
        for (int ii = 0; ii < 3; ii++) {
            resid(ii) = sigCart[ii];
        }
        jac.Multiply(diff, residguess);
        residguess += res0;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }
}

void TPZSandlerDimaggioPV::TaylorCheckDF2Cart(STATE theta, STATE beta, STATE k, TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm) const {
    STATE deltatheta = 0.4;
    STATE deltabeta = 0.05;
    STATE deltak = 0.5;
    TPZFNMatrix<3, STATE> res0(3, 1), resid(3, 1), residguess(3, 1), diff(3, 1);
    TPZFNMatrix<9, STATE> jac(3, 3);
    DF2Cart(theta, beta, k, jac);
    TPZManVector<STATE> sigHWCyl(3), sigCart(3);
    F2Cyl(theta, beta, k, sigHWCyl);
    FromHWCylToPrincipal(sigHWCyl, sigCart);
    for (int i = 0; i < 3; i++) {
        res0(i) = sigCart[i];
    }
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        STATE difftheta = deltatheta * i / 10.;
        STATE thetanext = theta + difftheta;
        STATE diffbeta = deltabeta * i / 10.;
        STATE betanext = beta + diffbeta;
        STATE diffk = deltak * i / 10.;
        STATE knext = k + diffk;
        diff(0) = difftheta;
        diff(1) = diffbeta;
        diff(2) = diffk;
        F2Cyl(thetanext, betanext, knext, sigHWCyl);
        FromHWCylToPrincipal(sigHWCyl, sigCart);
        for (int ii = 0; ii < 3; ii++) {
            resid(ii) = sigCart[ii];
        }
        jac.Multiply(diff, residguess);
        residguess += res0;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

void TPZSandlerDimaggioPV::TaylorCheckProjectSigma(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm) const {
    TPZManVector<STATE, 3> deltasigma(3, -0.000012), sigproj(3);
    deltasigma[1] = -4.e-6;
    deltasigma[2] = -4.e-6;
    //    TPZManVector<STATE,3> deltasigma(3,-0.01), sigproj(3);
    //    deltasigma[1] *= 3;
    //    deltasigma[2] *= -2;

    TPZFNMatrix<3, STATE> res0(3, 1), diff(3, 1), resid(3, 1), residguess(3, 1);
    TPZFNMatrix<9, STATE> jac(3, 3);
    STATE kproj;
    ProjectSigmaDep(sigtrial, kprev, sigproj, kproj, jac);
    for (int j = 0; j < 3; j++) res0(j) = sigproj[j];
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        TPZManVector<STATE, 3> diffsigma(3), nextsigma(3);
        for (int j = 0; j < 3; j++) {
            diffsigma[j] = deltasigma[j] * i / 10.;
            nextsigma[j] = sigtrial[j] + diffsigma[j];
            diff(j) = diffsigma[j];
        }
        jac.Multiply(diff, residguess);
        residguess += res0;
        ProjectSigma(nextsigma, kprev, sigproj, kproj);
        for (int j = 0; j < 3; j++) resid(j) = sigproj[j];
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }
}

/// verifies the validity of dxi/dsigtrial and dbeta/dsigtrial

void TPZSandlerDimaggioPV::TaylorCheckParamF1Sigtrial(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm) const {
    TPZManVector<STATE, 3> deltasigma(3, -0.01), sigproj(3);
    deltasigma[1] *= 3.;
    deltasigma[2] *= -2.;
    TPZFNMatrix<3, STATE> res0(2, 1), diff(3, 1), resid(2, 1), residguess(2, 1);
    TPZFNMatrix<9, STATE> jac(2, 3), jacF1(2, 2), gradF1(2, 3);
    STATE kproj;
    ProjectF1(sigtrial, kprev, sigproj, kproj);
    STATE xi, beta;
    SurfaceParamF1(sigproj, xi, beta);
    res0(0) = xi;
    res0(1) = beta;
    D2DistFunc1(sigtrial, xi, beta, jacF1);
    GradF1SigmaTrial(sigtrial, xi, beta, gradF1);
    //gradF1.Transpose();
    jacF1.Solve_LU(&gradF1);
    jac = -1. * gradF1;
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        TPZManVector<STATE, 3> diffsigma(3), nextsigma(3);
        for (int j = 0; j < 3; j++) {
            diffsigma[j] = deltasigma[j] * i / 10.;
            nextsigma[j] = sigtrial[j] + diffsigma[j];
            diff(j) = diffsigma[j];
        }
        jac.Multiply(diff, residguess);
        residguess += res0;
        ProjectF1(nextsigma, kprev, sigproj, kproj);
        SurfaceParamF1(sigproj, xi, beta);
        resid(0) = xi;
        resid(1) = beta;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

void TPZSandlerDimaggioPV::TaylorCheckProjectF1(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm) const {
    TPZManVector<STATE, 3> deltasigma(3, -0.01), sigproj(3);
    deltasigma[1] *= 3.;
    deltasigma[2] *= -2.;
    TPZFNMatrix<3, STATE> res0(3, 1), diff(3, 1), resid(3, 1), residguess(3, 1);
    TPZFNMatrix<9, STATE> jac(3, 3), jacF1(2, 2), gradF1(2, 3), DF1cart(2, 3), GradSigma(3, 3);
    STATE kproj;
    ProjectF1(sigtrial, kprev, sigproj, kproj);
    STATE xi, beta;
    SurfaceParamF1(sigproj, xi, beta);
    res0(0) = sigproj[0];
    ;
    res0(1) = sigproj[1];
    res0(2) = sigproj[2];
    D2DistFunc1(sigtrial, xi, beta, jacF1);
    GradF1SigmaTrial(sigtrial, xi, beta, gradF1);
    //gradF1.Transpose();
    jacF1.Solve_LU(&gradF1);
    DF1Cart(xi, beta, DF1cart);
    DF1cart.Multiply(gradF1, GradSigma);
    GradSigma *= -1.;

    jac = GradSigma;
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        TPZManVector<STATE, 3> diffsigma(3), nextsigma(3);
        for (int j = 0; j < 3; j++) {
            diffsigma[j] = deltasigma[j] * i / 10.;
            nextsigma[j] = sigtrial[j] + diffsigma[j];
            diff(j) = diffsigma[j];
        }
        jac.Multiply(diff, residguess);
        residguess += res0;
        ProjectF1(nextsigma, kprev, sigproj, kproj);
        resid(0) = sigproj[0];
        resid(1) = sigproj[1];
        resid(2) = sigproj[2];
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

void TPZSandlerDimaggioPV::TaylorCheckProjectF2(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm) const {
    //    TPZManVector<STATE,3> deltasigma(3,-0.01), sigproj(3);
    //    deltasigma[1] *= 3.;
    //    deltasigma[2] *= -2.;
    TPZManVector<STATE, 3> deltasigma(3, -0.000012), sigproj(3);
    deltasigma[1] = -4.e-6;
    deltasigma[2] = -4.e-6;
    TPZFNMatrix<3, STATE> res0(3, 1), diff(3, 1), resid(3, 1), residguess(3, 1);
    TPZFNMatrix<9, STATE> jac(3, 3), jacF2(3, 3), gradF2(3, 3), DF2cart(3, 3), GradSigma(3, 3);
    STATE kproj;
    ProjectF2(sigtrial, kprev, sigproj, kproj);
    STATE theta, beta;
    SurfaceParamF2(sigproj, kproj, theta, beta);
    res0(0) = sigproj[0];
    ;
    res0(1) = sigproj[1];
    res0(2) = sigproj[2];
    D2DistFunc2(sigtrial, theta, beta, kproj, jacF2);

    TFad<3, STATE> thetafad(theta, 0), betafad(beta, 1), kprojfad(kproj, 2);
    TPZManVector<TFad<3, STATE>, 3> sigtrialfad(3), ddistf2(3);
    for (int m = 0; m < 3; m++) {
        sigtrialfad[m] = sigtrial[m];
    }
    //DDistFunc2(sigtrialfad, thetafad, betafad, kprojfad, kprev, ddistf2);

    TPZFNMatrix<9, STATE> diffjac(3, 3);
    for (int m = 0; m < 3; m++) {
        for (int n = 0; n < 3; n++) {
            diffjac(m, n) = jacF2(m, n) - ddistf2[m].fastAccessDx(n);
            jacF2(m, n) = ddistf2[m].fastAccessDx(n);
        }
    }


    GradF2SigmaTrial(sigtrial, theta, beta, kproj, kprev, gradF2);
    //gradF1.Transpose();
    jacF2.Solve_LU(&gradF2);
    DF2Cart(theta, beta, kproj, DF2cart);
    DF2cart.Multiply(gradF2, GradSigma);
    GradSigma *= -1.;

    jac = GradSigma;
    xnorm.resize(10);
    errnorm.resize(10);
    for (int i = 1; i <= 10; i++) {
        TPZManVector<STATE, 3> diffsigma(3), nextsigma(3);
        for (int j = 0; j < 3; j++) {
            diffsigma[j] = deltasigma[j] * i / 10.;
            nextsigma[j] = sigtrial[j] + diffsigma[j];
            diff(j) = diffsigma[j];
        }
        jac.Multiply(diff, residguess);
        residguess += res0;
        ProjectF2(nextsigma, kprev, sigproj, kproj);
        resid(0) = sigproj[0];
        resid(1) = sigproj[1];
        resid(2) = sigproj[2];
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
    }

}

void TPZSandlerDimaggioPV::TaylorCheckDtbkDsigtrial(const TPZVec<STATE> &sigtrial, STATE kprev, TPZVec<STATE> &xnorm, TPZVec<STATE> &errnorm) const {
    //    TPZManVector<STATE,3> deltasigma(3,-0.01), sigproj(3);
    //    deltasigma[1] *= 3.;
    //    deltasigma[2] *= -2.;
    TPZManVector<STATE, 3> deltasigma(3, -0.000012), sigproj(3);
    deltasigma[1] = -4.e-6;
    deltasigma[2] = -4.e-6;
    TPZFNMatrix<3, STATE> res0(3, 1), diff(3, 1), resid(3, 1), residguess(3, 1);
    TPZFNMatrix<9, STATE> jac(3, 3), jacF2(3, 3), gradF2(3, 3), GradTBK(3, 3);
    STATE kproj;
    ProjectF2(sigtrial, kprev, sigproj, kproj);
    STATE theta, beta;
    SurfaceParamF2(sigproj, kproj, theta, beta);
    res0(0) = theta;
    res0(1) = beta;
    res0(2) = kproj;
    D2DistFunc2(sigtrial, theta, beta, kproj, jacF2);
    TFad<3, STATE> thetafad(theta, 0), betafad(beta, 1), kprojfad(kproj, 2);
    TPZManVector<TFad<3, STATE>, 3> sigtrialfad(3), ddistf2(3);
    for (int m = 0; m < 3; m++) {
        sigtrialfad[m] = sigtrial[m];
    }
    //DDistFunc2(sigtrialfad, thetafad, betafad, kprojfad, kprev, ddistf2);

    TPZFNMatrix<9, STATE> diffjac(3, 3);
    for (int m = 0; m < 3; m++) {
        for (int n = 0; n < 3; n++) {
            diffjac(m, n) = jacF2(m, n) - ddistf2[m].fastAccessDx(n);
            jacF2(m, n) = ddistf2[m].fastAccessDx(n);
        }
    }
    diffjac.Print("DiffMatrix");

    GradF2SigmaTrial(sigtrial, theta, beta, kproj, kprev, gradF2);
    //gradF1.Transpose();
    TPZFMatrix<STATE> jacF2temp(jacF2), gradF2temp(gradF2);
    jacF2temp.Solve_LU(&gradF2temp);
    GradTBK = gradF2temp;
    GradTBK *= -1.;

    jac = GradTBK;

    TPZFNMatrix<3, STATE> tbk(3, 1, 0.), rhs;
    tbk(1, 0) = 0.1;
    rhs = tbk;
    GradTBK.Solve_LU(&rhs);
    for (int i = 0; i < 3; i++) {
        deltasigma[i] = rhs(i, 0);
    }


    xnorm.resize(10);
    errnorm.resize(10);
    TPZFNMatrix<30, STATE> erros(3, 10);
    for (int i = 1; i <= 10; i++) {
        TPZManVector<STATE, 3> diffsigma(3), nextsigma(3);
        TPZFNMatrix<3, STATE> difftbk(3, 1);
        for (int j = 0; j < 3; j++) {
            diffsigma[j] = deltasigma[j] * i / 10.;
            difftbk(j) = tbk[j] * i / 10.;
            nextsigma[j] = sigtrial[j] + diffsigma[j];
            diff(j) = diffsigma[j];
        }
        jac.Multiply(diff, residguess);
        residguess += res0;
        //        TPZFNMatrix<3,STATE> multsig(3,0),multbk(3,0);
        //        jacF2.Multiply(difftbk, multbk);
        //        gradF2.Multiply(diff, multsig);
        //        residguess = multbk;
        //        residguess = multsig;
        //        TPZFNMatrix<3,STATE> fxn(3,1);
        //        DDistFunc2(nextsigma, theta+difftbk[0],beta+difftbk[1],kproj+difftbk[2],kprev,fxn);
        //        TPZManVector<STATE> fxnvec(3);
        //        DDistFunc2<STATE>(sigtrial,theta+difftbk[0],beta+difftbk[1],kproj+difftbk[2],kprev,fxnvec);
        //        for(int k=0; k<3; k++) fxn(k,0) = fxnvec[k];
        //        DDistFunc2(sigtrial, theta+difftbk[0],beta+difftbk[1],kproj+difftbk[2],kprev,fxn);
        //        DDistFunc2(nextsigma, theta,beta,kproj,kprev,fxn);
        ProjectF2(nextsigma, kprev, sigproj, kproj);
        STATE theta, beta;
        SurfaceParamF2(sigproj, kproj, theta, beta);
        resid(0) = theta;
        resid(1) = beta;
        resid(2) = kproj;
        xnorm[i - 1] = Norm(diff);
        errnorm[i - 1] = Norm(resid - residguess);
        for (int a = 0; a < 3; a++) erros(a, i - 1) = fabs(resid[a] - residguess[a]);
    }
    erros.Print(cout);

}

void TPZSandlerDimaggioPV::MCormicRanchSand(TPZSandlerDimaggioPV &mat)//em ksi
{
    STATE E = 100, nu = 0.25, A = 0.25, B = 0.67, C = 0.18, D = 0.67, R = 2.5, W = 0.066, psi = 1.0;
    STATE G = E / (2. * (1. + nu));
    STATE K = E / (3. * (1. - 2 * nu));
    mat.fA = A;
    mat.fB = B;
    mat.fC = C;
    mat.fD = D;
    mat.fK = K;
    mat.fG = G;
    mat.fW = W;
    mat.fR = R;
    mat.fE = E;
    mat.fnu = nu;
    mat.fElasticResponse.SetUp(E, nu);
}

void TPZSandlerDimaggioPV::ReservoirSandstone(TPZSandlerDimaggioPV &mat)//em ksi
{
    STATE E = 1305, nu = 0.25, A = 2.61, B = 0.169, C = 2.57, D = 0.05069, R = 1.5, W = 0.0908, psi = 1.0;
    STATE G = E / (2. * (1. + nu));
    STATE K = E / (3. * (1. - 2 * nu));
    mat.fA = A;
    mat.fB = B;
    mat.fC = C;
    mat.fD = D;
    mat.fK = K;
    mat.fG = G;
    mat.fW = W;
    mat.fR = R;
    mat.fE = E;
    mat.fnu = nu;
    mat.fElasticResponse.SetUp(E, nu);
}

void TPZSandlerDimaggioPV::SalemLimestone(TPZSandlerDimaggioPV &mat)// em MPa
{
    STATE E = 22547., nu = 0.2524, A = 689.2,
            B = 3.94e-4, C = 675.2, D = 1.47e-3, R = 28, W = 0.08, psi = 1.0;
    STATE G = E / (2. * (1. + nu));
    STATE K = E / (3. * (1. - 2 * nu));
    mat.fA = A;
    mat.fB = B;
    mat.fC = C;
    mat.fD = D;
    mat.fK = K;
    mat.fG = G;
    mat.fW = W;
    mat.fR = R;
    mat.fE = E;
    mat.fnu = nu;
    mat.fElasticResponse.SetUp(E, nu);
}

void TPZSandlerDimaggioPV::PreSMat(TPZSandlerDimaggioPV &mat)// em MPa
{

    STATE E = 29269, nu = 0.203, A = 116.67,
            B = 0.0036895, C = 111.48, D = 0.018768, R = 0.91969, W = 0.006605, psi = 1.0;
    STATE G = E / (2. * (1. + nu));
    STATE K = E / (3. * (1. - 2 * nu));
    mat.fA = A;
    mat.fB = B;
    mat.fC = C;
    mat.fD = D;
    mat.fK = K;
    mat.fG = G;
    mat.fW = W;
    mat.fR = R;
    mat.fE = E;
    mat.fnu = nu;
    mat.fElasticResponse.SetUp(E, nu);
}


//REAL E = 29269,
//poisson = 0.203;
//
//material.fER.SetUp(E, poisson);
//
//REAL A = 116.67,
//B = 0.0036895,
//C = 111.48,
//D = 0.018768,
//R = 0.91969,
//W = 0.006605;
