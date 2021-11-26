#include <iostream>
#include <vector>
#include <regex>
#include <fstream>
#include <cstdio>
#include <sstream>
using namespace std;

string regs[] = {"t1","t2","t3","t4","t5","t6","t7","t8","t9","s0","s1","s2","s3","s4","s5","s6","s7"};
vector<string> variables;
map<string,string> table;
map<string,int> reg_ok1;
map<string,int> reg_ok2;
int ismain = 1;
int func = 0;
int paranum = 0;
int para2 = 0;
vector<string> parameter;

void Load_Var(string Inter){
    regex temp_re("temp\\d+");
    smatch result;
    string temp_line;
    string::const_iterator iter = Inter.begin();
    string::const_iterator iterEnd = Inter.end();
    while (regex_search(iter,iterEnd,result,temp_re)){
        temp_line = result[0];
        //cout<<temp_line<<endl;
        variables.emplace_back(temp_line);
        iter = result[0].second;
    }
//    for(auto it = variables.begin();it!=variables.end();it++)
//        cout<<*it<<endl;
}

string Load_Inter(const string& filename){
    string lines;
    string temp_line;
    ifstream in(filename);
    if(in){
        while (getline(in,temp_line)){
            if (temp_line == " ")
                continue;
            lines.append(temp_line);
            lines.append("\n");
        }
    }
    in.close();
    return lines;
}

string Get_R(const string& temp_str){
    for (auto it = variables.begin();it!=variables.end();++it)
        if (*it == temp_str){
            it = variables.erase(it);
            break;
        }

    if (table.find(temp_str) != table.end())//如果已经存在寄存器分配，那么直接返回寄存器
        return "$"+table[temp_str];
    else{//没找到
        vector<string> keys;
        for (auto & it : table)//已经分配寄存器的变量key
            keys.emplace_back(it.first);

        for (auto & key : keys)//当遇到未分配寄存器的变量时，清空之前所有分配的临时变量的映射关系
            if (key.find("temp")!=string::npos && find(variables.begin(),variables.end(),key) == variables.end()){
                reg_ok1[table[key]] = 1;
                table.erase(key);
            }

        for (const auto & reg : regs) //对于所有寄存器
            if(reg_ok1[reg] || reg_ok2[reg]){ //如果寄存器可用
                table[temp_str] = reg;//将可用寄存器分配给该变量，映射关系存到table中
                reg_ok1[reg] = 0;//寄存器reg设置为已用
                reg_ok2[reg] = 0;//寄存器reg设置为已用
                return "$"+reg;
            }
    }
    return "";
}

bool strisdigit(string str){
    int i;
    for(i = 0; i < str.length()-1; i++){
        if(isdigit(str[i]) == false)
            return false;
    }
    return true;
}

string translate(string temp_str){
    //将每行string按空格存成数组
    vector<string> line;
    string temp_res;
    stringstream input(temp_str);
    while (input>>temp_res)
        line.emplace_back(temp_res);
    //核心处理
    string temp_return;
    if(line[0] == "LABEL")
        return line[1]+":";
    if (line[1] == "=") {
        if (line.size() == 3){
            //处理数字
            if (strisdigit(line[2])){
                return "\tli " + Get_R(line[0]) + ","+line.back();
            }
            else{
                temp_return = "\tmove ";
                temp_return += Get_R(line[0])+',';
                temp_return += Get_R(line[2]);
                return temp_return;
            }
        }
        if (line.size() == 5){

            if (line[3] == "+")
                if (strisdigit(line[4])){
                    temp_return = "\taddi ";
                    temp_return += Get_R(line[0])+",";
                    temp_return += Get_R(line[2])+",";
                    temp_return += line.back();
                    return temp_return;
                }
                else{
                    temp_return = "\tadd ";
                    temp_return += Get_R(line[0])+",";
                    temp_return += Get_R(line[2])+",";
                    temp_return += Get_R(line.back());
                    return temp_return;
                }
            else if (line[3] == "-"){
                if (strisdigit(line[4])){
                    temp_return = "\tsubi ";
                    temp_return += Get_R(line[0])+",";
                    temp_return += Get_R(line[2])+",";
                    temp_return += line.back();
                    return temp_return;
                }
                else{
                    temp_return = "\tsub ";
                    temp_return += Get_R(line[0])+",";
                    temp_return += Get_R(line[2])+",";
                    temp_return += Get_R(line.back());
                    return temp_return;
                }
            }
            else if (line[3] == "*"){
                temp_return = "\tmul ";
                temp_return += Get_R(line[0])+",";
                temp_return += Get_R(line[2])+",";
                temp_return += Get_R(line.back());
                return temp_return;
            }
            else if (line[3] == "/"){
                temp_return = "\tdiv ";
                temp_return += Get_R(line[2])+",";
                temp_return += Get_R(line.back()) + "\n\tmflo ";
                temp_return += Get_R(line[0]);
                return temp_return;
            }
        }

        if (line[2] == "CALL"){
            if (line[3] == "read" || line[3] == "print"){
                //func = 1;
                return "\taddi $sp,$sp,-4\n\tsw $ra,0($sp)\n\tjal " + line.back() + "\n\tlw $ra,0($sp)\n\tmove " +Get_R(line[0]) + ",$v0\n\taddi $sp,$sp,4";
            }
            else{
                func = 1;
                reg_ok2["t0"] = 1;
                reg_ok2["ra"] = 1;
                reg_ok2["t1"] = 1;
                reg_ok2["t2"] = 1;
                reg_ok2["t3"] = 1;
                reg_ok2["t4"] = 1;
                char c = 4*para2+'0';
                return "\taddi $sp,$sp,-24\n\tsw $t0,0($sp)\n\tsw $ra,4($sp)\n\tsw $t1,8($sp)\n\tsw $t2,12($sp)\n\tsw $t3,16($sp)\n\tsw $t4,20($sp)\n\tjal "+line.back()+"\n\tlw $a0,0($sp)\n\tlw $ra,4($sp)\n\tlw $t1,8($sp)\n\tlw $t2,12($sp)\n\tlw $t3,16($sp)\n\tlw $t4,20($sp)\n\taddi $sp,$sp,24"+"\n\taddi $sp,$sp," + c;
            }
        }
    }

    if (line[0] == "GOTO")
        return "\tj "+line[1];
    if (line[0] == "RETURN"){
        if(ismain){
            return "\tli $v0, 10\n\tsyscall";
        }
        else if(strisdigit(line[1])){
            ismain = 1;
            return "\tli $v0, "+line[1]+"\n\tjr $ra";
        }   
        else{
            ismain = 1;
            return "\tmove $v0,"+Get_R(line[1])+"\n\tjr $ra";
        }
            
    }
    if (line[0] == "IF") {
        if (line[2] == "=="){
            temp_return = "\tbeq ";
            temp_return += Get_R(line[1])+",";
            if(strisdigit(line[3]))
                temp_return += line[3]+",";
            else
                temp_return += Get_R(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == "!="){
            temp_return = "\tbne ";
            temp_return += Get_R(line[1])+",";
            if(strisdigit(line[3]))
                temp_return += line[3]+",";
            else
                temp_return += Get_R(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == ">"){
            temp_return = "\tbgt ";
            temp_return += Get_R(line[1])+",";
            if(strisdigit(line[3]))
                temp_return += line[3]+",";
            else
                temp_return += Get_R(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == "<"){
            temp_return = "\tblt ";
            temp_return += Get_R(line[1])+",";
            if(strisdigit(line[3]))
                temp_return += line[3]+",";
            else
                temp_return += Get_R(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == ">="){
            temp_return = "\tbge ";
            temp_return += Get_R(line[1])+",";
            if(strisdigit(line[3]))
                temp_return += line[3]+",";
            else
                temp_return += Get_R(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
        if (line[2] == "<="){
            temp_return = "\tble ";
            temp_return += Get_R(line[1])+",";
            if(strisdigit(line[3]))
                temp_return += line[3]+",";
            else
                temp_return += Get_R(line[3])+",";
            temp_return += line.back();
            return temp_return;
        }
    }
    if (line[0] == "FUNCTION"){
        if(line[1] != "main")
            ismain = 0;
        return line[1]+":";
    }
    if (line[0] == "CALL")
        if (line.back() == "read" || line.back() == "write"){
            func = 2;
            return "\taddi $sp,$sp,-4\n\tsw $ra,0($sp)\n\tjal "+line.back()+"\n\tlw $ra,0($sp)\n\taddi $sp,$sp,4";
        }
        else{
            reg_ok2["t0"] = 1;
            reg_ok2["ra"] = 1;
            reg_ok2["t1"] = 1;
            reg_ok2["t2"] = 1;
            reg_ok2["t3"] = 1;
            reg_ok2["t4"] = 1;
            func = 1;
            char c = 4*para2+'0';
            return "\taddi $sp,$sp,-24\n\tsw $t0,0($sp)\n\tsw $ra,4($sp)\n\tsw $t1,8($sp)\n\tsw $t2,12($sp)\n\tsw $t3,16($sp)\n\tsw $t4,20($sp)\n\tjal "+line.back()+"\n\tlw $a0,0($sp)\n\tlw $ra,4($sp)\n\tlw $t1,8($sp)\n\tlw $t2,12($sp)\n\tlw $t3,16($sp)\n\tlw $t4,20($sp)\n\taddi $sp,$sp,24\n\tmove "+Get_R(line[0])+" $v0\n\taddi $sp,$sp," + c;
        }
    if (line[0] == "ARG"){
    	para2++;
        return "\taddi $sp,$sp,-4\n\tsw " + Get_R(line.back()) +",0($sp)" + "\n\tmove $a0," + Get_R(line.back());
        // return "\tmove $t0,$a0\n\tmove $a0,"+Get_R(line.back());
    }
    if (line[0] == "PARAM"){
        paranum++;
        parameter.push_back(line[1]);  
        // table[line.back()] = "a0";
    }
    return " ";
}

void write_to_txt(const vector<string>& obj){
    ofstream out("Ningyu.asm");
    string temp =".data\n"
                     "_prompt: .asciiz \"Enter an integer:\"\n"
                     "_ret: .asciiz \"\\n\"\n"
                     ".globl main\n"
                     ".text\n"
                     
                     "j main\n"
                     "read:\n"
                     "    li $v0,4\n"
                     "    la $a0,_prompt\n"
                     "    syscall\n"
                     "    li $v0,5\n"
                     "    syscall\n"
                     "    jr $ra\n"
                     "\n"
                     "write:\n"
                     "    li $v0,1\n"
                     "    syscall\n"
                     "    li $v0,4\n"
                     "    la $a0,_ret\n"
                     "    syscall\n"
                     "    move $v0,$0\n"
                     "    jr $ra\n"
                     ;
    out<<temp;
    for (auto & it:obj)
        out<<it<<endl;
    out.close();
}

string num_to_string(int n)
{
	string res;
	stringstream ss;
	ss << n;
	ss >> res;
	return res;
}

int main(){
    for (const auto & reg : regs) //初始化，所有寄存器都可用
    {    
        reg_ok1[reg] = 1;
        reg_ok2[reg] = 1;
	}
    string filename = "optimized.txt";
    string Inter = Load_Inter(filename);//读取中间代码

    Load_Var(Inter);//第一遍扫描，记录所有变量
    //翻译
    vector<string> obj;
    ifstream in(filename);
    string temp_line,obj_line;
    if(in) {
        while (getline(in, temp_line)) {
            obj_line = translate(temp_line);
            if (obj_line == " ")
                continue;
            
            if(paranum > 0)
            {
            	int base = 20 + 4 * paranum;
            	paranum = 0;
            	for(string s:parameter)
            	{
            		obj.emplace_back("\tlw "+Get_R(s) +"," + num_to_string(base) +"($sp)\t");
            		base -= 4;
            	}	
            }
            
            obj.emplace_back(obj_line);
            if(func)
            {
                reg_ok2["t0"] = 0;
                reg_ok2["ra"] = 0;
                reg_ok2["t1"] = 0;
                reg_ok2["t2"] = 0;
                reg_ok2["t3"] = 0;
                reg_ok2["t4"] = 0;
                string line_0 = temp_line.substr(0,temp_line.find(' '));
                if(func == 1)
                    obj.emplace_back("\tmove "+Get_R(line_0)+",$v0");
                func = 0;
                para2 = 0;
            }
        }
    } else{
        cout<<"file open falied.\n";
    }
    in.close();
    write_to_txt(obj);
    return 0;
}
