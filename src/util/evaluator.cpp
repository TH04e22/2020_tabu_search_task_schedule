#include "../include/evaluator.h"
#include <iostream>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <ctime>
#include <random>
#include <bitset>
using namespace std;

void ignoreSectionMsg( ifstream& file ) {
    if( !file.is_open() ) {
        cout << "The file not yet open" << endl;
        throw runtime_error("The file isn't opened!");
    }

    if( !file.fail()) {
        while( file.peek() != '/' && !file.fail() ) {
            file.ignore(numeric_limits<streamsize>::max(), '\n');
        };

        do {
            file.ignore(numeric_limits<streamsize>::max(), '\n');
        } while( file.peek() != '*' && !file.fail() );
        file.ignore(numeric_limits<streamsize>::max(), '\n');

        if( file.fail() ) 
            throw runtime_error( "Invalid section message format!" );
    }
}

Evaluator::Evaluator()
: thePCount(0), theTCount(0), theECount(0) {

}

Evaluator::Evaluator( string const& filename ) : Evaluator() {
    readConfig( filename );
}

Evaluator::~Evaluator() {

}

void Evaluator::getInitStr( std::vector<unsigned int> &ss, std::vector<unsigned int> &ms ) const{ // 獲得初始字串
    shuffle_order_engine<default_random_engine, 256> re( time(NULL) );
    uniform_int_distribution<unsigned int> int_dis( 0, thePCount-1 );
    uniform_real_distribution<> real_dis(0, 1);

    ms.resize( theTCount );
    for( unsigned int& item : ms ) // 隨機匹配字串
        item = int_dis(re);

    ss = initStr;
    if( ss.size() > 3 ) { // 生成隨機排程字串
        for( int i = 2; i < theTCount - 1; i++ ) {
            if( real_dis(re) > 0.5 && !isSuccessor( ss[i], ss[i-1] ) ) {
                swap( ss[i],ss[i-1]);                
            }
        }
    }
}

bool Evaluator::isSuccessor( unsigned int i, unsigned int j ) const { // 工作i是否是工作j的後繼者
    unsigned int bytes = j / 8;
    unsigned int bits = j % 8;
    unsigned char answer = taskPredecessor[i][bytes] << bits & 0x80;
    return ( answer == 0x80 ) ? true : false; 
}

void Evaluator::readConfig( string const& filename ) {
    ifstream file( filename, ios::in );
    this->filename = filename;
    if( !file.is_open() ) {
        cerr << "The file can't open" << endl;
    } else {
        double value;
        /* 讀取區段1 thePCount、theTCount、theECount */
        ignoreSectionMsg( file );
        file >> thePCount >> theTCount >> theECount;
        
        /* 讀取區段2 double[pFrom][pTo] theCommRate*/
        ignoreSectionMsg( file );
        theCommRate.resize( thePCount );
        for( int i = 0; i < thePCount && !file.fail(); i++ ) {
            theCommRate[i].resize( thePCount );
            for( int j = 0; j < thePCount && !file.fail() ; j++ ) {
                file >> value;
                theCommRate[i][j] = value;
            }
        }

        /* 讀取區段3 double[tID][pID] theCompCost*/
        ignoreSectionMsg( file );
        theCompCost.resize( theTCount );
        for( int i = 0; i < theTCount && !file.fail(); i++ ) {
            theCompCost[i].resize( thePCount );
            for( int j = 0; j < thePCount && !file.fail(); j++ ) {
                file >> value;
                theCompCost[i][j] = value;
            }
        }

        /* 讀取區段4 double[tFrom][tTo] theTransDataVol*/
        int fromTask, toTask;
        double cost;
        vector<int> task_count( theTCount, 0 ); // 計算各task的入分支度
        ignoreSectionMsg( file );
        theTransDataVol.resize( theTCount );

        for( int i = 0; i < theTCount; i++ ) {
            theTransDataVol[i].clear();
        }

        for( int i = 0; i < theECount && !file.fail(); i++ ) {
            file >> fromTask >> toTask >> cost;
            task_count[toTask]++;
            theTransDataVol[fromTask].push_front(TransData(toTask,cost));
        }

        if( file.fail() ) {
            thePCount = theECount = theTCount = 0;
            theCommRate.clear();
            theCompCost.clear();
            theTransDataVol.clear();
            initStr.clear();
            taskPredecessor.clear();
            throw runtime_error( "Invalid file data format!" );
        }

        if( theTCount > 0 ) { // get initial string and compute initial schedueling string
            int i, j, k, top = -1;
            unsigned int arr_size = theTCount / 8 + 1;
            initStr.resize( theTCount );
            taskPredecessor.resize( theTCount );
            for( auto& t : taskPredecessor )
                t.assign( arr_size, 0 );
            
            for( i = 0; i < theTCount; i++) {
                if( task_count[i] == 0 ) {
                    swap(top, task_count[i]);
                }
            }

            for( i = 0; i < theTCount; i++ ) {
                if( top == -1 ) {
                    initStr.clear();
                    taskPredecessor.clear();
                    throw runtime_error( "Invalid secheduel: Job network has a cycle!");
                } else {
                    j = top;
                    initStr[i] = j; // set up initial schedueling string
                    top = task_count[top];

                    unsigned int bytes = j / 8;
                    unsigned int bits = j % 8;
                    unsigned char bitMask = 0x80 >> bits; // b10000000

                    for( TransData &succ : theTransDataVol[j] ) {
                        k = succ.toTask;
                        task_count[k]--;
                        taskPredecessor[succ.toTask][bytes] |= bitMask;
                        for( int b = 0; b < arr_size; b++ ) {
                            taskPredecessor[succ.toTask][b] |= taskPredecessor[j][b]; 
                        }

                        if( !task_count[k] ) {
                            task_count[k] = top;
                            top = k;
                        }
                    }
                }
            }
        }
    }
}

double Evaluator::getCost( const vector<unsigned int>& ss, const vector<unsigned int>& ms ) { // ms : 匹配字串 ss : 排程字串
    p_ft.assign( thePCount, 0 );
    st.assign( theTCount, 0 );
    ft.assign( theTCount, -1 );

    for( auto task_i = ss.begin(); task_i != ss.end(); task_i++ ) {
        if( ft[*task_i] != -1 )
            throw invalid_argument("Duplicated task is scheduled in Schedule String!");

        if( st[*task_i] < p_ft[ms[*task_i]] ) {
            st[*task_i] = p_ft[ms[*task_i]];
        }
        p_ft[ms[*task_i]] = ft[*task_i] = st[*task_i] + theCompCost[*task_i][ms[*task_i]];

        for( TransData &succ : theTransDataVol[*task_i] ) {
            if( ft[succ.toTask] != -1 )
                throw invalid_argument("Invalid Sequence in Schedule String!");

            double start_time = ft[*task_i] + theCommRate[ms[*task_i]][ms[succ.toTask]] * succ.commCost;

            if( start_time > st[succ.toTask] ) {
                st[succ.toTask] = start_time;
            }
        }   
    }
    return ft.back();
}

void Evaluator::printConfig() const {
    /* 列印出 thePCount, theTCount, theECount */
    cout << "---- ----Section 1---- ----" << endl;
    cout << "Numbers of Processor: " << thePCount
         << "\nNumbers of Task: " << theTCount
         << "\nNumbers of Edges: " << theECount << endl;
    
    /* 列印出 theCommRate */
    cout << "---- ----Section 2---- ----" << endl;
    cout << "Processor Communication Rate:" << endl;
    for( auto first_it = theCommRate.begin(); first_it != theCommRate.end(); first_it++ ) {
        for( auto second_it = (*first_it).begin(); second_it != (*first_it).end(); second_it++ ) {
            cout << *second_it << ' ';
        }
        cout << endl;
    }

    /* 列印出 theCompCost */
    cout << "---- ----Section 3---- ----" << endl;
    cout << "Task to Processor Computation Cost:" << endl;
    for( auto first_it = theCompCost.begin(); first_it != theCompCost.end(); first_it++ ) {
        for( auto second_it = (*first_it).begin(); second_it != (*first_it).end(); second_it++ ) {
            cout << *second_it << ' ';
        }
        cout << endl;
    }

    /* 列印出 theTransDataVol, 亦即各個工作之間有向邊的資訊 */
    cout << "---- ----Section 4---- ----" << endl;
    cout << "TransData between Tasks:" << endl;
    cout << "from to cost" << endl;
    int count = 0;
    for( auto first_it = theTransDataVol.begin(); first_it != theTransDataVol.end(); first_it++ ) {
        for( auto second_it = (*first_it).begin(); second_it != (*first_it).end(); second_it++ ) {
            cout << count << " " << second_it->toTask << " " << second_it->commCost << endl;
        }
        count++;
    }

    /* 列印出初始字串、工作優先權表格 */
    cout << "---- ----The precedence table---- ----" << endl;
    for( int i = 0; i < theTCount; i++ ) {
        cout << "task" << i << ": ";
        for( const unsigned char &c : taskPredecessor[i] ) {
            bitset<8> b(c);
            cout << b << " ";
        }
        cout << endl;
    }

    cout << "---- ----Initial String---- ----" << endl;
    cout << "Schedueling String:" << endl;
    for( auto &task : initStr ) {
        cout << task << " ";
    }
    cout << "\n-----------------------------" << endl;
}

void Evaluator::printTaskExecTime() const {
    cout << "Task start time:" << endl;
    for( auto it = st.begin(); it != st.end(); it++ ) {
        cout << *it << " ";
    }

    cout << "\n\nTask finish time:" << endl;
    for( auto it = ft.begin(); it != ft.end(); it++ ) {
        cout << *it << " ";
    }
    cout << endl;
}
