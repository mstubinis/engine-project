#include "Engine.h"
#include "ShaderProgram.h"
#include "Material.h"
#include "Camera.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Scene.h"

#include <boost/filesystem.hpp>
#include <regex>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace std;

bool sfind(string whole,string part){ if(whole.find(part) != string::npos) return true; return false; }
void insertStringAtLine(string& src, const string& newcontent,uint line){   
    if(line == 0){src=newcontent+"\n"+src;}else{istringstream str(src);string l; vector<string> lines;uint count=0;
        while(getline(str,l)){lines.push_back(l+"\n");if(count==line){lines.push_back(newcontent+"\n");}++count;}src="";for(auto ln:lines){src+=ln;}
    }
}
void insertStringAtEndOfMainFunc(string& src, const string& content){
    uint p=src.size()-1;while(p>0){char c=src.at(p);--p;if(c=='}'){break;}}src.insert(p,content);
}
void insertStringRightAfterLineContent(string& src, const string& newContent,const string& lineContent){
    istringstream str(src);string l; vector<string> lines; bool a = false;
    while(getline(str,l)){lines.push_back(l+"\n");if(sfind(l,lineContent) && !a){lines.push_back(newContent+"\n"); a=true;}}src="";for(auto ln:lines){src+=ln;}
}

string getLogDepthFunctions(){
    string res =  "\n"
        "vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float log_depth = (texture2D(gDepthMap, _uv).r);\n"
        "    float regularDepth = pow(_far + 1.0, log_depth) - 1.0;\n"//log to regular depth
        "    float a = _far / (_far - _near);\n"
        "    float b = _far * _near / (_near - _far);\n"
        "    float linearDepth = (a + b / regularDepth);\n"
        "    vec4 clipSpace = vec4(_uv,linearDepth, 1.0) * 2.0 - 1.0;\n"
        "    vec4 wpos = CameraInvViewProj * clipSpace;\n"
        "    return wpos.xyz / wpos.w;\n"
        "}//log depth\n"
        "vec3 GetViewPosition(vec2 _uv,float _near, float _far){//generated\n"
        "    float depth = texture2D(gDepthMap, _uv).r;\n"
        "    depth = pow(_far + 1.0, depth) - 1.0;\n"//log to regular depth
        "    float a = _far / (_far - _near);\n"//linearize regular depth
        "    float b = _far * _near / (_near - _far);\n"
        "    float linearDepth = (a + b / depth);\n"
        "    vec4 clipSpace = CameraInvProj * vec4(_uv,linearDepth, 1.0) * 2.0 - 1.0;\n"
        "    return clipSpace.xyz / clipSpace.w;\n"
        "}//log depth\n";
    return res;
}
string getNormalDepthFunctions(){
    string res = "\n"
        "vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated\n"
		"	 vec4 clipSpace = vec4(vec2(_uv * 2.0 - 1.0),texture2D(gDepthMap, _uv).r,1.0);\n"
		"	 vec4 worldPos = CameraInvViewProj * clipSpace;\n"
		"	 return worldPos.xyz / worldPos.w;\n"
        "}//normal depth\n"
        "vec3 GetViewPosition(vec2 _uv,float _near, float _far){//generated\n"
		"	 vec4 clipSpace = vec4(vec2(_uv * 2.0 - 1.0),texture2D(gDepthMap, _uv).r,1.0);\n"
		"	 vec4 viewPos = CameraInvProj * clipSpace;\n"
		"	 return viewPos.xyz / viewPos.w;\n"
        "}//normal depth\n";
    return res;
}

UniformBufferObject* UniformBufferObject::UBO_CAMERA = nullptr;

namespace Engine{
    namespace epriv{
        struct DefaultShaderBindFunctor{void operator()(EngineResource* r) const {
            Scene* s = Resources::getCurrentScene(); if(!s) return;
            Camera* c = s->getActiveCamera();        if(!c) return;

            float fcoeff = (2.0f / glm::log2(c->getFar() + 1.0f)) * 0.5f;
            Renderer::sendUniform1fSafe("fcoeff",fcoeff);

            if(Renderer::Settings::GodRays::enabled()) Renderer::sendUniform1iSafe("HasGodsRays",1);
            else                                       Renderer::sendUniform1iSafe("HasGodsRays",0);
        }};
        struct DefaultShaderUnbindFunctor{void operator()(EngineResource* r) const {
        }};
        struct srtKey{inline bool operator() ( Material* _1,  Material* _2){return (_1->name() < _2->name());}};
    };
};
GLint UniformBufferObject::MAX_UBO_BINDINGS;
uint UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT = 0;

class Shader::impl final{
    public:
        ShaderType::Type m_Type;
        bool m_FromFile;
        string m_Data;
        void _init(string& data, ShaderType::Type type, bool fromFile,Shader* super){
            m_Data = data;
            m_Type = type;
            m_FromFile = fromFile;
            if(fromFile){
                super->setName(data);
            }
            else{
                super->setName("NULL");
            }
        }
        void _destruct(){
        }

};
Shader::Shader(string shaderFileOrData, ShaderType::Type shaderType,bool fromFile):m_i(new impl){ m_i->_init(shaderFileOrData,shaderType,fromFile,this); }
Shader::~Shader(){ m_i->_destruct(); }
ShaderType::Type Shader::type(){ return m_i->m_Type; }
string Shader::data(){ return m_i->m_Data; }
bool Shader::fromFile(){ return m_i->m_FromFile; }

epriv::DefaultShaderBindFunctor DEFAULT_BIND_FUNCTOR;
epriv::DefaultShaderUnbindFunctor DEFAULT_UNBIND_FUNCTOR;
class ShaderP::impl final{
    public:
        ShaderRenderPass::Pass m_Stage;
        GLuint m_ShaderProgram;
        vector<Material*> m_Materials;
        unordered_map<string,GLint> m_UniformLocations;
        unordered_map<GLuint,bool> m_AttachedUBOs;
        Shader* m_VertexShader;
        Shader* m_FragmentShader;

        void _convertCode(string& _data1,Shader* shader1,string& _data2,Shader* shader2,ShaderP* super){ _convertCode(_data1,shader1,super); _convertCode(_data2,shader2,super); }
        void _convertCode(string& _d,Shader* shader,ShaderP* super){
            istringstream str(_d); 
            
            //see if we actually have a version line
            string versionLine;
            if(sfind(_d,"#version ")){
                //use the found one
                while(true){
                    getline(str,versionLine); if(sfind(versionLine,"#version ")){ break; }
                }
            }
            else{
                //generate one
                string core = "";
                if(epriv::RenderManager::GLSL_VERSION >= 330) core = " core";
                versionLine = "#version " + boost::lexical_cast<string>(epriv::RenderManager::GLSL_VERSION) + core + "\n";
                insertStringAtLine(_d,versionLine,0);
            }

            string versionNumberString = regex_replace(versionLine,regex("([^0-9])"),"");
            uint versionNumber = boost::lexical_cast<uint>(versionNumberString);

            //check for normal map texture extraction
            //refer to mesh.cpp dirCorrection comment about using an uncompressed normal map and not reconstructing z
            if(sfind(_d,"CalcBumpedNormal(") || sfind(_d,"CalcBumpedNormalCompressed(")){
                if(!sfind(_d,"vec3 CalcBumpedNormal(vec2 _uv,sampler2D _inTexture){//generated")){
                    if(sfind(_d,"varying mat3 TBN;")){
                        boost::replace_all(_d,"varying mat3 TBN;","");
                    }
                    string normalMap = 
                    "varying mat3 TBN;\n"
                    "vec3 CalcBumpedNormal(vec2 _uv,sampler2D _inTexture){//generated\n"			
                    "    vec3 _t = (texture2D(_inTexture, _uv).xyz) * 2.0 - 1.0;\n"
                    "    return normalize(TBN * _t);\n"
                    "}\n"
                    "vec3 CalcBumpedNormalCompressed(vec2 _uv,sampler2D _inTexture){//generated\n"
                    "    vec2 _t = (texture2D(_inTexture, _uv).yx) * 2.0 - 1.0;\n" //notice the yx flip, its needed
                    "    float _z = sqrt(1.0 - _t.x * _t.x - _t.y * _t.y);\n"
                    "    vec3 normal = vec3(_t.xy, _z);\n"//recalc z in the shader
                    "    return normalize(TBN * normal);\n"
                    "}\n";
                    insertStringAtLine(_d,normalMap,1);
                }
            }
            //check for painters algorithm
            if(sfind(_d,"PaintersAlgorithm(")){
                if(!sfind(_d,"vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){//generated")){
                    string painters = "\n"
                    "vec4 PaintersAlgorithm(vec4 paint, vec4 canvas){//generated\n"
                    "    float paintA = paint.a;\n"
                    "    float canvasA = canvas.a;\n"
                    "    float Alpha = paintA + canvasA * (1.0 - paint.a);\n"
                    "    vec3 r = (paint.rgb * paintA + canvas.rgb * canvasA * (1.0 - paintA)) / Alpha;\n"
                    "    return vec4(r,Alpha);\n"
                    "}\n";
                    insertStringAtLine(_d,painters,1);
                }
            }

            //see if we need a UBO for the camera
            if(sfind(_d,"CameraView") || sfind(_d,"CameraProj") || sfind(_d,"CameraViewProj") || sfind(_d,"CameraInvView") || sfind(_d,"CameraInvProj") || 
            sfind(_d,"CameraInvViewProj") || sfind(_d,"CameraPosition") || sfind(_d,"CameraNear") || sfind(_d,"CameraFar") || sfind(_d,"CameraInfo1") ||
            sfind(_d,"CameraInfo2") || sfind(_d,"CameraViewVector")){
                string uboCameraString;
                if(versionNumber >= 140){ //UBO
                     if(!sfind(_d,"layout (std140) uniform Camera //generated")){
                         uboCameraString = "\n"
                         "layout (std140) uniform Camera //generated\n"
                         "{\n"
                         "    mat4 CameraView;\n"
                         "    mat4 CameraProj;\n"
                         "    mat4 CameraViewProj;\n"
                         "    mat4 CameraInvView;\n"
                         "    mat4 CameraInvProj;\n"
                         "    mat4 CameraInvViewProj;\n"
                         "    vec4 CameraInfo1;\n"
                         "    vec4 CameraInfo2;\n"
                         "};\n"
                         "vec3 CameraPosition = CameraInfo1.xyz;\n"
                         "vec3 CameraViewVector = CameraInfo2.xyz;\n"
                         "float CameraNear = CameraInfo1.w;\n"
                         "float CameraFar = CameraInfo2.w;\n"
                         "\n";
                         insertStringAtLine(_d,uboCameraString,1);
                     }
                }
                else{ //no UBO's, just add a uniform struct
                    if(!sfind(_d,"uniform mat4 CameraView;//generated")){
                         uboCameraString = "\n"
                         "uniform mat4 CameraView;//generated;\n"
                         "uniform mat4 CameraProj;\n"
                         "uniform mat4 CameraViewProj;\n"
                         "uniform mat4 CameraInvView;\n"
                         "uniform mat4 CameraInvProj;\n"
                         "uniform mat4 CameraInvViewProj;\n"
                         "uniform vec4 CameraInfo1;\n"
                         "uniform vec4 CameraInfo2;\n"
                         "vec3 CameraPosition = CameraInfo1.xyz;\n"
                         "vec3 CameraViewVector = CameraInfo2.xyz;\n"
                         "float CameraNear = CameraInfo1.w;\n"
                         "float CameraFar = CameraInfo2.w;\n"
                         "\n";
                         insertStringAtLine(_d,uboCameraString,1);
                    }
                }	
            }

            //check for log depth - vertex
            if(sfind(_d,"USE_LOG_DEPTH_VERTEX") && !sfind(_d,"//USE_LOG_DEPTH_VERTEX") && shader->type() == ShaderType::Vertex){
                boost::replace_all(_d,"USE_LOG_DEPTH_VERTEX","");
                #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                string log_vertex_code = "\n"
                    "uniform float fcoeff;\n"
                    "flat varying float FC;\n"
                    "varying float logz_f;\n"
                    "\n";
                insertStringAtLine(_d,log_vertex_code,1);
                log_vertex_code = "\n"
                    "logz_f = 1.0 + gl_Position.w;\n"
                    "gl_Position.z = (log2(max(1e-6, logz_f)) * fcoeff - 1.0) * gl_Position.w;\n"
                    "FC = fcoeff;\n"
                    "\n";
                insertStringAtEndOfMainFunc(_d,log_vertex_code);
                #endif
            }
            //check for log depth - fragment
            if(sfind(_d,"USE_LOG_DEPTH_FRAGMENT") && !sfind(_d,"//USE_LOG_DEPTH_FRAGMENT") && shader->type() == ShaderType::Fragment){
                boost::replace_all(_d,"USE_LOG_DEPTH_FRAGMENT","");
                #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                string log_frag_code = "\n"
                    "flat varying float FC;\n"
                    "varying float logz_f;\n"
                    "\n";
                insertStringAtLine(_d,log_frag_code,1);
                log_frag_code = "\n"
                    "gl_FragDepth = log2(logz_f) * FC;\n"
                    "\n";
                insertStringAtEndOfMainFunc(_d,log_frag_code);
                #endif
                if(sfind(_d,"GetWorldPosition(") || sfind(_d,"GetViewPosition(")){
                    
                    if(!sfind(_d,"vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated")){
                        #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                             insertStringRightAfterLineContent(_d,getLogDepthFunctions(),"uniform sampler2D gDepthMap;");
                        #else
                             insertStringRightAfterLineContent(_d,getNormalDepthFunctions(),"uniform sampler2D gDepthMap;");
                        #endif
                    }

                }
            }
            else{
                if(sfind(_d,"GetWorldPosition(") || sfind(_d,"GetViewPosition(")){
                    if(!sfind(_d,"vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated")){
                        if(sfind(_d,"USE_LOG_DEPTH_FRAG_WORLD_POSITION") && !sfind(_d,"//USE_LOG_DEPTH_FRAG_WORLD_POSITION") && shader->type() == ShaderType::Fragment){
                            //log
                            boost::replace_all(_d,"USE_LOG_DEPTH_FRAG_WORLD_POSITION","");
                            if(!sfind(_d,"vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated")){
                                #ifndef ENGINE_FORCE_NO_LOG_DEPTH
                                     insertStringRightAfterLineContent(_d,getLogDepthFunctions(),"uniform sampler2D gDepthMap;");
                                #else
                                     insertStringRightAfterLineContent(_d,getNormalDepthFunctions(),"uniform sampler2D gDepthMap;");
                                #endif
                            }
                        }
                        else{
                            //normal
                            if(!sfind(_d,"vec3 GetWorldPosition(vec2 _uv,float _near, float _far){//generated")){
                                insertStringRightAfterLineContent(_d,getNormalDepthFunctions(),"uniform sampler2D gDepthMap;");
                            }
                        }
                    }
                }
            }




            if(versionNumber >= 130){
                if(shader->type() == ShaderType::Vertex){
                    boost::replace_all(_d, "varying", "out");
                }
                else if(shader->type() == ShaderType::Fragment){
                    boost::replace_all(_d, "varying", "in");
                    boost::replace_all(_d, "gl_FragColor", "FRAG_COL");
                    insertStringAtLine(_d,"out vec4 FRAG_COL;",1);
                }
            }
            if(versionNumber >= 140){
                if(shader->type() == ShaderType::Vertex){
                }
                else if(shader->type() == ShaderType::Fragment){
                    boost::replace_all(_d, "textureCube(", "texture(");
                    boost::replace_all(_d, "textureCubeLod(", "textureLod(");
                    boost::replace_all(_d, "texture2DLod(", "textureLod(");
                    boost::replace_all(_d, "texture2D(", "texture(");
                }
            }	
            if(versionNumber >= 150){
                if(shader->type() == ShaderType::Vertex){
                }
                else if(shader->type() == ShaderType::Fragment){
                }
            }
            if(versionNumber >= 330){
                if(shader->type() == ShaderType::Vertex){
                }
                else if(shader->type() == ShaderType::Fragment){
                }
            }
        }
        void _init(string& name, Shader* vs, Shader* fs, ShaderRenderPass::Pass stage,ShaderP* super){
            m_Stage = stage;
            m_VertexShader = vs;
            m_FragmentShader = fs;

            super->setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
            super->setName(name);
            epriv::Core::m_Engine->m_RenderManager->_addShaderToStage(super,stage);
            
            string& _name = super->name();
            if(vs->name() == "NULL") vs->setName(_name + ".vert");
            if(fs->name() == "NULL") fs->setName(_name + ".frag");
            _compileOGL(m_VertexShader,m_FragmentShader,_name,super);
        }
        void _destruct(){
            glDeleteShader(m_ShaderProgram);
            glDeleteProgram(m_ShaderProgram);
            m_UniformLocations.clear();
            m_AttachedUBOs.clear();
        }
        void _compileOGL(Shader* vs,Shader* fs,string& _shaderProgramName,ShaderP* super){
            m_UniformLocations.clear();
            m_AttachedUBOs.clear();
            GLuint vid=glCreateShader(GL_VERTEX_SHADER);GLuint fid=glCreateShader(GL_FRAGMENT_SHADER);
            string VertexCode,FragmentCode = "";
            if(vs->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str(vs->data());
                for(string line;getline(str,line,'\n');){VertexCode+="\n"+line;}
            }
            else{VertexCode=vs->data();}
            if(fs->fromFile()){
                boost::iostreams::stream<boost::iostreams::mapped_file_source> str1(fs->data());
                for(string line;getline(str1,line,'\n');){FragmentCode+="\n"+line; }
            }
            else{FragmentCode=fs->data();}

            _convertCode(VertexCode,vs,FragmentCode,fs,super);

            GLint res=GL_FALSE; int ll;

            // Compile Vertex Shader
            char const* vss=VertexCode.c_str();glShaderSource(vid,1,&vss,NULL);glCompileShader(vid);

            // Check Vertex Shader
            glGetShaderiv(vid,GL_COMPILE_STATUS,&res);glGetShaderiv(vid,GL_INFO_LOG_LENGTH,&ll);vector<char>ve(ll);glGetShaderInfoLog(vid,ll,NULL,&ve[0]);

            if(res==GL_FALSE){
                if(vs->fromFile()){cout<<"VertexShader Log ("+vs->data()+"): "<<endl;}
                else{cout<<"VertexShader Log ("+vs->name()+"): "<<endl;}
                cout<<&ve[0]<<endl;
            }

            // Compile Fragment Shader
            char const* fss=FragmentCode.c_str();glShaderSource(fid,1,&fss,NULL);glCompileShader(fid);

            // Check Fragment Shader
            glGetShaderiv(fid,GL_COMPILE_STATUS,&res);glGetShaderiv(fid,GL_INFO_LOG_LENGTH,&ll);vector<char>fe(ll);glGetShaderInfoLog(fid,ll,NULL,&fe[0]);

            if(res==GL_FALSE){
                if(fs->fromFile()){cout<<"FragmentShader Log ("+fs->data()+"): "<<endl;}
                else{cout<<"FragmentShader Log ("+fs->name()+"): "<<endl;}
                cout<<&fe[0]<<endl;
            }

            // Link the program id
            m_ShaderProgram=glCreateProgram();
            glAttachShader(m_ShaderProgram,vid);glAttachShader(m_ShaderProgram,fid);
            glLinkProgram(m_ShaderProgram);
            glDetachShader(m_ShaderProgram,vid);glDetachShader(m_ShaderProgram,fid);
            glDeleteShader(vid);glDeleteShader(fid);

            // Check the program
            glGetProgramiv(m_ShaderProgram,GL_LINK_STATUS,&res);glGetProgramiv(m_ShaderProgram,GL_INFO_LOG_LENGTH,&ll);vector<char>pe(std::max(ll,int(1)));
            glGetProgramInfoLog(m_ShaderProgram,ll,NULL,&pe[0]);

            if(res==GL_FALSE){cout<<"ShaderProgram Log : "<<endl;cout<<&pe[0]<<endl;}
         
            //populate uniform table
            if(res==GL_TRUE){
                GLint _i,_count,_size;
                GLenum _type; // type of the variable (float, vec3 or mat4, etc)
                const GLsizei _bufSize = 256; // maximum name length
                GLchar _name[_bufSize]; // variable name in GLSL
                GLsizei _length; // name length
                glGetProgramiv(m_ShaderProgram,GL_ACTIVE_UNIFORMS,&_count);
                for(_i=0;_i<_count;++_i){
                    glGetActiveUniform(m_ShaderProgram,(GLuint)_i,_bufSize,&_length,&_size,&_type,_name);
                    if(_length>0){
                        string _name1((char*)_name,_length);
                        GLint _loc = glGetUniformLocation(m_ShaderProgram,_name);
                        m_UniformLocations.emplace(_name1,_loc);
                    }
                }
            }
            if(sfind(VertexCode,"layout (std140) uniform Camera //generated") || sfind(FragmentCode,"layout (std140) uniform Camera //generated")){
                UniformBufferObject::UBO_CAMERA->attachToShader(super);
            }
        }
};
ShaderP::ShaderP(string n, Shader* vs, Shader* fs, ShaderRenderPass::Pass s):m_i(new impl){ m_i->_init(n,vs,fs,s,this); }
ShaderP::~ShaderP(){ m_i->_destruct(); }
GLuint ShaderP::program(){ return m_i->m_ShaderProgram; }
ShaderRenderPass::Pass ShaderP::stage(){ return m_i->m_Stage; }
vector<Material*>& ShaderP::getMaterials(){ return m_i->m_Materials; }

void ShaderP::bind(){
    epriv::Core::m_Engine->m_RenderManager->_bindShaderProgram(this);
    BindableResource::bind();
}
void ShaderP::unbind(){
    BindableResource::unbind();
}
void ShaderP::addMaterial(Handle& materialHandle){
    ShaderP::addMaterial(Resources::getMaterial(materialHandle));
}
void ShaderP::addMaterial(Material* material){
    m_i->m_Materials.push_back(material);
    sort(m_i->m_Materials.begin(),m_i->m_Materials.end(),epriv::srtKey());
}
const unordered_map<string,GLint>& ShaderP::uniforms() const { return this->m_i->m_UniformLocations; }


class UniformBufferObject::impl final{
    public:
        const char* nameInShader;
        uint sizeOfStruct;
        int globalBindingPointNumber;
        GLuint uboObject;
        void _init(const char* _nameInShader,uint& _sizeofStruct,int _globalBindingPointNumber){
            nameInShader = _nameInShader;
            if(epriv::RenderManager::GLSL_VERSION < 140) return;
            if(_globalBindingPointNumber == -1){
                //automatic assignment
                globalBindingPointNumber = (UniformBufferObject::MAX_UBO_BINDINGS-1) - UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
                ++UniformBufferObject::CUSTOM_UBO_AUTOMATIC_COUNT;
                if(globalBindingPointNumber < 0){
                    cout << "Warning: Max UBO Limit reached!" << std::endl;
                    globalBindingPointNumber = 0;
                }
            }
            else{
                globalBindingPointNumber = _globalBindingPointNumber;
            }
            sizeOfStruct = _sizeofStruct;

            glGenBuffers(1, &uboObject);
            glBindBuffer(GL_UNIFORM_BUFFER, uboObject);//gen and bind buffer
            glBufferData(GL_UNIFORM_BUFFER, sizeOfStruct, NULL, GL_DYNAMIC_DRAW); //create buffer data storage
            glBindBuffer(GL_UNIFORM_BUFFER, 0); //is this really needed?

            glBindBufferBase(GL_UNIFORM_BUFFER, globalBindingPointNumber, uboObject);//link UBO to it's global numerical index
        }
        void _destruct(){
            if(epriv::RenderManager::GLSL_VERSION < 140) return;
            glDeleteBuffers(1,&uboObject);
        }
        void _update(void* _data){
            if(epriv::RenderManager::GLSL_VERSION < 140) return;
            glBindBuffer(GL_UNIFORM_BUFFER, uboObject);
            glBufferSubData(GL_UNIFORM_BUFFER,0, sizeOfStruct, _data);
            glBindBuffer(GL_UNIFORM_BUFFER, 0); //is this really needed?
        }
        void _attachToShader(UniformBufferObject* super,ShaderP* _shaderProgram){
            GLuint program = _shaderProgram->program();
            if(epriv::RenderManager::GLSL_VERSION < 140 || _shaderProgram->m_i->m_AttachedUBOs.count(program)) return;
            uint programBlockIndex = glGetUniformBlockIndex(program,nameInShader);
            glUniformBlockBinding(program, programBlockIndex, globalBindingPointNumber);
            _shaderProgram->m_i->m_AttachedUBOs.emplace(program,true);
        }
};
UniformBufferObject::UniformBufferObject(const char* _nameInShader,uint _sizeofStruct,int _globalBindingPointNumber):m_i(new impl){ m_i->_init(_nameInShader,_sizeofStruct,_globalBindingPointNumber); }
UniformBufferObject::~UniformBufferObject(){ m_i->_destruct(); }
void UniformBufferObject::updateData(void* _data){ m_i->_update(_data); }
void UniformBufferObject::attachToShader(ShaderP* _shaderProgram){ m_i->_attachToShader(this,_shaderProgram); }
