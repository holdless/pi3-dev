/*
1. on project righ-click > properties > configuration properties > vc directories > set all include/lib dependencies for Java
2. Java SE version depends on the Visual Studio version (32 or 64), not the operation system version. wrong version causes "unsolved external..."
3. remember to add "jvm.lib"
*/

// c_call_java.cpp : Defines the entry point for the console application.
//

#include <stdio.h> // instead of stdafx.h in VC++
#include <stdlib.h> // malloc
#include <iostream>
#include <string.h>
#include <jni.h>
using namespace std;

jstring str2jstring(JNIEnv* env,const char* pat);
std::string jstring2str(JNIEnv* env, jstring jstr);

int main(int argc, char* argv[])
{
    JavaVM *jvm;       /* denotes a Java VM */
    JNIEnv *env;       /* pointer to native method interface */
    JavaVMInitArgs vm_args; /* JDK/JRE 8 VM initialization arguments */
    JavaVMOption* options = new JavaVMOption[1];
    options[0].optionString = "-Djava.class.path=/home/pi/Desktop/arena/pi/pos-tagging/jar/StanfordPosTagger/StanfordPosTagger.jar";
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 1;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;
    /* load and initialize a Java VM, return a JNI interface
     * pointer in env */
    JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
    delete options;
    /* invoke the Main.test method using the JNI */
    jclass cls = env->FindClass("stanfordpostagger/StanfordPosTagger");
	
    // call java method "init" to initialize the stanford pos tagger model
    jmethodID mid = env->GetStaticMethodID(cls, "init", "(Ljava/lang/String;)Ledu/stanford/nlp/tagger/maxent/MaxentTagger;");
    const char *path = "/home/pi/Desktop/arena/pi/pos-tagging/jar/StanfordPosTagger/models/english-left3words-distsim.tagger";
    // tranform c-string to java-string (since string in Java is an object)
    jstring jpath = str2jstring(env, path);
    jobject tagger = env->CallStaticObjectMethod(cls, mid, jpath);

    // call java method "tag" to pos-tag sentence
    jmethodID mid2 = env->GetStaticMethodID(cls, "tag", "(Ledu/stanford/nlp/tagger/maxent/MaxentTagger;Ljava/lang/String;)I");
    const char *sentence = "i am jack, how are you?";
    // tranform c-string to java-string (since string in Java is an object)
    jstring jsentence = str2jstring(env, sentence);
    int a = env->CallStaticIntMethod(cls, mid2, tagger, jsentence);

    jvm->DestroyJavaVM();
    return 0;
}

jstring str2jstring(JNIEnv* env,const char* pat)
{
    // Define java String class "strClass"
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    // get String(byte[],String) constructor to tranform local byte[] array to a new string.
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    // new a byte array
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    // transform char* into byte array
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
    // set String, preserve language class for the parameters of byte array transform to String
    jstring encoding = (env)->NewStringUTF("GB2312"); 
    // transform byte array into java String, then output
    return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);
}
 
 
std::string jstring2str(JNIEnv* env, jstring jstr)
{   
    char* rtn = NULL;   
    jclass clsstring = env->FindClass("java/lang/String");   
    jstring strencode = env->NewStringUTF("GB2312");   
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");   
    jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);   
    jsize alen = env->GetArrayLength(barr);   
    jbyte* ba = env->GetByteArrayElements(barr,JNI_FALSE);   
    if(alen > 0)   
    {   
        rtn = (char*)malloc(alen + 1);         
        memcpy(rtn,ba,alen);   
        rtn[alen] = 0;   
    }   
    env->ReleaseByteArrayElements(barr,ba,0);   
    std::string stemp(rtn);
    free(rtn);
    return   stemp;   
} 

/*
JNIEXPORT jstring JNICALL Java_JavaHowTo_sayHello(JNIEnv *env, jobject obj, string str) 
{
    return  env->NewStringUTF(str);
}
*/
