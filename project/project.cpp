#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <limits>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

// remove space
string removeSpaces(const string& s) {
    string res;
    for (char c : s)
        if (c != ' ')
            res += c;
    return res;
}

// تحليل جزء من المعادلة (يمين أو يسار
void parseSide(const string& s, vector<double>& coef, double& constant, int sideSign) {
    if (s.empty()) return;

    int i = 0;
    while (i < s.size()) {
        int sign = 1;
        if (s[i] == '+') {
            sign = 1;
            i++;
        }
        else if (s[i] == '-') {
            sign = -1;
            i++;
        }

        double num = 0;
        bool hasNum = false;
        bool decimalFound = false;
        double decimalMultiplier = 0.1;

        // قراءة الرقم
        while (i < s.size() && (isdigit(s[i]) || s[i] == '.')) {
            if (s[i] == '.') {
                decimalFound = true;
                i++;
                continue;
            }
            if (!decimalFound) {
                num = num * 10 + (s[i] - '0');
            }
            else {
                num = num + (s[i] - '0') * decimalMultiplier;
                decimalMultiplier *= 0.1;
            }
            hasNum = true;
            i++;
        }

        // التحقق من وجود متغير بعد الرقم
        if (i < s.size() && s[i] == 'x') {
            i++;
            int varIndex = 0;
            // قراءة رقم المتغير (مثل x1, x2)
            if (i < s.size() && isdigit(s[i])) {
                while (i < s.size() && isdigit(s[i])) {
                    varIndex = varIndex * 10 + (s[i] - '0');
                    i++;
                }
            }
            else {
                // إذا كان x فقط بدون رقم، نعتبره x1
                varIndex = 1;
            }

            // التأكد من حجم المصفوفة
            if (varIndex >= coef.size()) {
                coef.resize(varIndex + 1, 0);
            }

            // إضافة المعامل (إذا لم يكن هناك رقم، المعامل = 1)
            if (!hasNum) num = 1;
            coef[varIndex] += sideSign * sign * num;

        }
        else {
            // ثابت
            if (hasNum) {
                constant += sideSign * sign * num;
            }
        }
    }
}

// دالة لطباعة معادلة مبسطة
void printSimplifiedEquation(const vector<double>& coef, double constant, int eqNum) {
    cout  << eqNum << " after : ";

    bool first = true;
    // طباعة المتغيرات
    for (int i = 1; i < coef.size(); i++) {
        if (fabs(coef[i]) > 1e-12) { // تجاهل القيم الصغيرة جداً
            if (!first && coef[i] > 0) cout << "+";

            // طباعة المعامل (إذا كان 1 أو -1 نكتفي بالإشارة)
            if (fabs(coef[i] - 1.0) < 1e-12) {
                cout << "x" << i;
            }
            else if (fabs(coef[i] + 1.0) < 1e-12) {
                cout << "-x" << i;
            }
            else {
                cout << coef[i] << "x" << i;
            }
            first = false;
        }
    }

    // طباعة الثابت
    if (fabs(constant) > 1e-12) {
        if (constant > 0) cout << "+" << constant;
        else cout << constant;
    }
    else if (first) {
        // إذا كان كل شيء صفراً
        cout << "0";
    }

    cout << " = 0" << endl;
}

int main() {
    int n;
    cout << "Enter number of equations: ";
    cin >> n;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<vector<double>> A; // مصفوفة المعاملات
    vector<double> B;         // المصفوفة الثابتة (الجانب الأيمن)
    int maxVar = 0;            // أكبر رقم متغير تم العثور عليه
    vector<int> varCounts;     // تخزين عدد المتغيرات في كل معادلة

    for (int eqNum = 0; eqNum < n; eqNum++) {
        string eq;
        cout << "Enter equation " << eqNum + 1 << ": ";
        getline(cin, eq);
        eq = removeSpaces(eq);

        // إضافة "=0" إذا لم يكن موجود
        int pos = eq.find('=');
        if (pos == string::npos) {
            eq += "=0";
            pos = eq.find('=');
        }

        vector<double> coef(1, 0); // نبدأ بالمؤشر 0 (غير مستخدم)
        double constant = 0;        // الثوابت من الجانبين

        // تحليل الطرفين
        parseSide(eq.substr(0, pos), coef, constant, +1);
        parseSide(eq.substr(pos + 1), coef, constant, -1);

        // حساب عدد المتغيرات في هذه المعادلة
        int varCount = 0;
        for (int i = 1; i < coef.size(); i++) {
            if (fabs(coef[i]) > 1e-12) varCount++;
        }
        varCounts.push_back(varCount);

        // طباعة المعادلة المبسطة
        printSimplifiedEquation(coef, constant, eqNum + 1);
        cout << "variables: " << varCount << "\n" << endl;

        // تحديث أكبر متغير
        maxVar = max(maxVar, (int)coef.size() - 1);

        // تجهيز المصفوفة: نقل coef إلى A و -constant إلى B
        // (لأن المعادلة تصبح A * X = -constant)
        A.push_back(coef);
        B.push_back(-constant);
    }

    // توحيد أبعاد المصفوفة A: maxVar+1 أعمدة (من 0 إلى maxVar)
    for (auto& row : A) {
        row.resize(maxVar + 1, 0);
    }


    // **حذف غاوسي مع اختيار المحور الرئيسي (Gaussian Elimination with Partial Pivoting)**
    vector<double> X(maxVar, 0); // حلول المتغيرات x1..xMaxVar

    for (int col = 1; col <= maxVar; col++) { // الأعمدة من 1 إلى maxVar
        // 1. اختيار المحور الرئيسي (Pivot) من الصفوف col-1 إلى n-1
        int pivotRow = -1;
        double maxVal = 0;
        for (int row = col - 1; row < n; row++) {
            if (row >= A.size()) break;
            if (fabs(A[row][col]) > maxVal + 1e-12) {
                maxVal = fabs(A[row][col]);
                pivotRow = row;
            }
        }

        // إذا كان المحور صفراً (أو قريباً من الصفر)، ننتقل للعمود التالي
        if (pivotRow == -1 || fabs(A[pivotRow][col]) < 1e-12) {
            continue;
        }

        // 2. تبديل الصفوف إذا لزم الأمر
        if (pivotRow != col - 1) {
            swap(A[col - 1], A[pivotRow]);
            swap(B[col - 1], B[pivotRow]);
        }

        // 3. تطبيع الصف (جعل المحور = 1)
        double pivot = A[col - 1][col];
        if (fabs(pivot) > 1e-12) {
            for (int j = col; j <= maxVar; j++) {
                A[col - 1][j] /= pivot;
            }
            B[col - 1] /= pivot;
            A[col - 1][col] = 1.0; // للتأكد
        }

        // 4. إزالة المتغير من الصفوف الأخرى (Gauss-Jordan)
        for (int row = 0; row < n; row++) {
            if (row == col - 1) continue;

            double factor = A[row][col];
            if (fabs(factor) < 1e-12) continue;

            for (int j = col; j <= maxVar; j++) {
                A[row][j] -= factor * A[col - 1][j];
            }
            B[row] -= factor * B[col - 1];
            A[row][col] = 0; // للتأكد
        }
    }

    // استخراج الحلول
    // بعد Gauss-Jordan، إذا كان المتغير موجوداً (أي معامله 1 في صف ما)، يكون الحل في B لنفس الصف
    vector<bool> varFound(maxVar, false);

    for (int col = 1; col <= maxVar; col++) {
        for (int row = 0; row < n; row++) {
            if (fabs(A[row][col] - 1.0) < 1e-10) {
                // تأكد أن باقي المعاملات في هذا الصف تساوي صفر
                bool valid = true;
                for (int j = 1; j <= maxVar; j++) {
                    if (j != col && fabs(A[row][j]) > 1e-10) {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    X[col - 1] = B[row];
                    varFound[col - 1] = true;
                    break;
                }
            }
        }
    }

    // طباعة النتائج
    cout << "\nSolutions:\n";
    for (int i = 1; i <= maxVar; i++) {
        cout << "x" << i << " = ";
        if (varFound[i - 1]) {
            cout << fixed << setprecision(4) << X[i - 1] << endl;
        }
        else {
            cout << "Free variable or no unique solution" << endl;
        }
    }

    return 0;
}