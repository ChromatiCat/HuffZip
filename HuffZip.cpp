#include <iostream>
#include <fstream> 
#include <string.h>
#include <string>
#include <cstdio>
#include <queue>
#include <map>

using namespace std;

struct HuffNode
{
	unsigned char uch;//字符 
	int freq;//频数 
	string code;//编码 
	HuffNode *lchild,*rchild,*parent;//指向左右孩子或父亲 
	HuffNode(){lchild=rchild=parent=NULL;}
}; 
typedef HuffNode* node_ptr;

class HuffZip
{
	private:
		fstream in_file,out_file;
		unsigned long freq[256]={0};
		node_ptr Array[256];//叶子数组 
		node_ptr root;
		int size;//字符数量 
		int chlen=0;
		map<unsigned char,string> table;
		class Compare
	    {
	        public:
	            bool operator()(const node_ptr& c1, const node_ptr& c2) const
	            {
	                return (*c1).freq > (*c2).freq;
	            }
	    };
	    priority_queue<node_ptr,vector<node_ptr>,Compare> pq;
	    void coding(const node_ptr node, bool left)
		{
		    if(left)
		        node->code = node->parent->code + "0";
		    else
		        node->code = node->parent->code + "1";
		
		    if(node->lchild == NULL && node->rchild == NULL)
		        table[node->uch] = node->code;
		    else
		    {
		        if(node->lchild != NULL)
		            coding(node->lchild, true);
		        if(node->rchild != NULL)
		            coding(node->rchild, false);
		    }
		}
	public:
		HuffZip(string in_file_name, string out_file_name)
		{
		    in_file.open(in_file_name.c_str(), ios::in|ios::binary);
		    if(!in_file)
		    {
		        cout<<"Open file error:"<<in_file_name<<endl;
		        exit(1);
		    }
		
		    out_file.open(out_file_name.c_str(), ios::out|ios::binary);
		    if(!out_file)
		    {
		        cout<<"Open file error:"<<out_file_name<<endl;
		        exit(1);
		    }
		}
		~HuffZip()
		{
		    in_file.close();
		    out_file.close();
		}
		void compress()
		{
			//统计字符频数 
			char ch;
			while(!in_file.eof())
			{
				in_file.read(&ch,sizeof(char));//每次读8bit(一个char) 
		        if( in_file.eof() )
		            break;
		        freq[(unsigned char)ch]++;//频数加1 
		        chlen++;//字符总数加1 
			}
			//建立叶子数组 
			int count=0;
			for(int i=0;i<256;i++)
			{
				if(freq[i]<=0)continue;
				node_ptr p=new HuffNode();
				p->uch=i;
				p->freq=freq[i];
				Array[count++]=p;
			}
			size=count; 
			//建立优先队列 
			for(int i=0;i<size;i++)
			{
				pq.push(Array[i]);
			}
			//建立哈夫曼树
			 root = NULL;
		
		    while( !pq.empty() )
		    {       
		        node_ptr first = pq.top();
		        pq.pop();
		        if( pq.empty() )
		        {
		            root = first;
		            break;
		        }
		        node_ptr second = pq.top();
		        pq.pop();
		        node_ptr new_node = new HuffNode();
		        new_node->freq = first->freq + second->freq;
		
		        if(first->freq <= second->freq)
		        {
		            new_node->lchild = first;
		            new_node->rchild = second;
		        }
		        else
		        {
		            new_node->lchild = second;
		            new_node->rchild = first;
		        }
		        first->parent = new_node;
		        second->parent = new_node;
		
		        pq.push(new_node);
		    }
		    //建立编码表
			if(root == NULL)
		    {
		        cout<<"Unknown error"<<endl;
		        exit(1);
		    }
		
		    if(root->lchild != NULL)
		        coding(root->lchild, true);
		    if(root->rchild != NULL)
		        coding(root->rchild, false); 
		        
		    //写入输出文件
		    out_file.write((char *)&chlen,sizeof(int));//写入字符总数 
			out_file.write((char *)&size,sizeof(int));//写入字符种类数 
			for(int i=0;i<size;i++)//写入各字符频数 
			{
				out_file.write((char *)&(Array[i]->uch),sizeof(char));
				out_file.write((char *)&(Array[i]->freq),sizeof(int));
			}
			string buff;
			in_file.clear();
   			in_file.seekg(ios::beg);
			while(!in_file.eof())
			{
				in_file.read(&ch,sizeof(char));//读取一个字符 
		        if( in_file.eof() )
		            break;
		        buff+=table[(unsigned char)ch];//在缓存字符串尾加上对应的哈夫曼编码 
		        int k=buff.length();
		        char buff_char=0;
		        while(k>=8)// 缓存字符串长度大于8时转换为二进制写入文件 
				{
					for(int i=0;i<8;i++)
					{
						if(buff[i]=='1')
						{
							buff_char=(buff_char<<1|1) ;
						}
						else
						{
							buff_char=buff_char<<1;
						}
					}
		        	out_file.write(&buff_char,sizeof(char));
		        	buff=buff.substr(8,k-8);
		        	k=buff.length();
				}
			}
			int k=buff.length();
	        char buff_char=0;

			for(int i=0;i<k;i++)
			{
				if(buff[i]=='1')
				{
					buff_char=(buff_char<<1|1) ;
				}
				else
				{
					buff_char=buff_char<<1;
				}
			}
			for(int i=k;i<8;i++)
			{
				buff_char=buff_char<<1;
			}
        	out_file.write(&buff_char,k);
		}
		void decompress()
		{
			in_file.read((char *)&chlen,sizeof(int));
			in_file.read((char *)&size,sizeof(int));
			int count=0; 
			for(int i=0;i<size;i++)
			{
				node_ptr p=new HuffNode();
				in_file.read((char *)&(p->uch),sizeof(char));
				in_file.read((char *)&(p->freq),sizeof(int));
				Array[count++]=p;
			}
			//建立优先队列 
			for(int i=0;i<size;i++)
			{
				pq.push(Array[i]);
			}
			//建立哈夫曼树
			 root = NULL;
		
		    while( !pq.empty() )
		    {       
		        node_ptr first = pq.top();
		        pq.pop();
		        if( pq.empty() )
		        {
		            root = first;
		            break;
		        }
		        node_ptr second = pq.top();
		        pq.pop();
		        node_ptr new_node = new HuffNode();
		        new_node->freq = first->freq + second->freq;
		
		        if(first->freq <= second->freq)
		        {
		            new_node->lchild = first;
		            new_node->rchild = second;
		        }
		        else
		        {
		            new_node->lchild = second;
		            new_node->rchild = first;
		        }
		        first->parent = new_node;
		        second->parent = new_node;
		
		        pq.push(new_node);
		    }
		    node_ptr node=root;
		    int chc=0;
		    while(!in_file.eof())
		    {
		    	unsigned char buff_char,och;
		    	in_file.read((char*)&buff_char,sizeof(char));//读取8bits 
		    	unsigned char flag=0x80;
		    	for(int i = 0; i < 8; ++i)//每一位判断是1还是0 
		        {
		
		            if(buff_char & flag)//1向右 
		                node = node->rchild;
		            else//0向左 
		                node = node->lchild;
					
		            if(node->lchild == NULL && node->rchild == NULL)// 遇到叶子节点向文件输出对应字符 
		            {
		                och = node->uch;
		                out_file.write((char *)&(och),sizeof(char));
		                chc++;
		                node=root;
		            }
		            
		            flag = flag >> 1;
		        }
			}
			outloop:;
		}
};

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        cout<<"Unknown Command"<<endl;
        exit(1);
    }

    if(0 == strcmp("-c", argv[1]))//压缩 
    {
        HuffZip h(argv[2], argv[3]);
        h.compress();
    }
    else if(0 == strcmp("-d", argv[1]))//解压 
    {
        HuffZip h(argv[2], argv[3]);
        h.decompress();
    }
    else
    {
        cout<<"Unknown Command"<<endl;
    }
    return 0;
}
