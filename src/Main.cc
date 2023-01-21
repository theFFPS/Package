#include <Data.hh>
#include <map>
#include <fstream>
#include <filesystem>
#include <IO.hh>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <sstream>
using namespace TNB;
using namespace std::filesystem;
map<string, string> defaults = {
    { "name", "_" }, { "version", "1.0.0-SNAPSHOT" }, { "description", "_" },
    { "author", "_" }, { "license", "_" }, { "checksum", "_" },
    { "package_id", "_" }, { "level", "_" }, { "build", "_" } 
};
string Resize(string text, int perBlock) {
    string out;
    for (int i = 0; i < (int)text.size(); i++) {
        if (text[i] % perBlock == 0) { for (int n = 0; n < perBlock; n++) out += text[i] / perBlock; out += '\0'; }
        else {
            char defaultC = text[i] / perBlock;
            char extra = text[i] % perBlock;
            for (int n = 0; n < perBlock; n++) out += defaultC;
            out += extra;
        }
    }
    return out;
}
string Desize(string text, int perBlock) {
    string out;
    for (int i = 0; i < (int)text.size(); i++) {
        char toAdd = 0;
        for (int n = 0; n < perBlock+1; n++) { toAdd += text[i]; i++; }
        i--;
        out += toAdd;
    }
    return out;
}
void MoveFile(string from, string to) {
    ifstream fI (from);
    string toWriteB;
    while(fI) toWriteB += fI.get();
    fI.close();
    remove(from);
    ofstream fO (to);
    string toWrite;
    for (size_t i = 0; i < toWriteB.size(); i++) if (i != toWriteB.size()-2 && i != toWriteB.size()-1) toWrite += toWriteB[i];
    fO << toWrite;
    fO.close();
}
int main(int argc, char **argv) { 
    string arg;
    if (argc < 2) arg = "help";
    else arg = argv[1];
    if (arg == "package") {
        if (argc < 3) { cout << "Usage: " << argv[0] << " package <DIR>" << endl; return -1; }
        string dir = argv[2];
        ifstream f (dir + "/project.tnb");
        if (! f.good()) { cout << "Setup project firstly!" << endl; return -1; }
        f.close();
        string code = ReadFile(dir + "/project.tnb");
        TNBParser parser = TNBParser(dir + "/project.tnb");
        string files; ifstream fil (dir + "/packageData/data.tgz"); while (fil) files += fil.get(); fil.close();
        string toWrite = Resize(ToBytes(code), 2) + string({ (char)126 }) + files;
        ofstream pkgFile (dir + "/" + parser.Get("name").GetString() + "@" + parser.Get("version").GetString() + "-" + parser.Get("build").GetString() + ".pkg");
        pkgFile << toWrite;
        pkgFile.close();
    } else if (arg == "project") {
        if (argc < 4) { cout << "Usage: " << argv[0] << " project <DIR> set/get/get-all/available-fields <FIELD> <VALUE>" << endl; return -1; }
        string dir = argv[2];
        string fn = argv[3];
        if (fn == "set") {
            if (argc < 6) { cout << "Usage: " << argv[0] << " project <DIR> set <FIELD> <VALUE>" << endl; return -1; }
            string field = argv[4];
            string value = argv[5];
            ifstream f (dir + "/project.tnb");
            TNBWriter writer;
            if (! f.good()) for (auto val : defaults) { if (val.first != field) writer.Push(val.first, (string)val.second); else writer.Push(field, (string)value); }
            else {
                TNBParser parser = TNBParser(dir + "/project.tnb");
                for (string tag : parser.GetTags()) { if (tag != field) writer.Push(tag, (string)parser.Get(tag).GetString()); else writer.Push(field, (string)value); }
            }
            writer.Write(dir + "/project.tnb");
            f.close();
        } else if (fn == "available-fields") {
            cout << "FIELD           DEFAULT" << endl;
            cout << "name                _" << endl;
            cout << "version      1.0.0-SNAPSHOT" << endl;
            cout << "description         _" << endl;
            cout << "author              _" << endl;
            cout << "license             _" << endl;
            cout << "checksum (sha-256)  _" << endl;
            cout << "package_id          _" << endl;
            cout << "build               _" << endl;
            cout << "level               _" << endl;
        } else if (fn == "get") {
            if (argc < 5) { cout << "Usage: " << argv[0] << " project <DIR> get <FIELD>" << endl; return -1; }
            string field = argv[4];
            ifstream f (dir + "/project.tnb");
            if (! f.good()) { cout << "Setup project firstly!" << endl; return -1; }
            f.close();
            TNBParser parser = TNBParser(dir + "/project.tnb");
            for (string tag : parser.GetTags()) if (tag == field) cout << parser.Get(field).GetString() << endl; 
        } else if (fn == "get-all") {
            ifstream f (dir + "/project.tnb");
            if (! f.good()) { cout << "Setup project firstly!" << endl; return -1; }
            f.close();
            TNBParser parser = TNBParser(dir + "/project.tnb");
            for (string tag : parser.GetTags()) cout << tag << ": " << parser.Get(tag).GetString() << endl; 
        } else { cout << "Usage: " << argv[0] << " project <DIR> set/get/get-all/available-fields <FIELD> <VALUE>" << endl; return -1; }
    } else if (arg == "remove") {
        if (argc < 3) { cout << "Usage: " << argv[0] << " remove <PACKAGES>" << endl; cout << "Use '" << argv[0] << " list-packages installed' to view installed packages!" << endl; return 0; }
        list<string> packagesToRemove;
        int removed = 0;
        for (int i = 2; i < argc; i++) packagesToRemove.push_back(string(argv[i]));
        for (string pkg : packagesToRemove) {
            ifstream f ("/Apps/" + pkg);
            if (!f.good()) { cout << "Package '" << pkg << "' not installed!" << endl; continue; }
            f.close();
            removed++;
            cout << "Package '" << pkg << "' removed!" << endl;
            system(("rm -rf /Apps/" + pkg).c_str());
            ifstream fP ("/Apps/packages.tnb");
            if (!fP.good()) {
                TNBWriter writer;
                writer.Push("id", (Array)Array());
                writer.Push("name", (Array)Array());
                writer.Push("version", (Array)Array());
                writer.Push("build", (Array)Array());
                writer.Write("/Apps/packages.tnb");
            } else {
                Array name;
                Array id;
                Array version;
                Array build;
                size_t removedPackageAt = 0;
                TNBWriter writer;
                TNBParser parser = TNBParser("/Apps/packages.tnb");
                for (size_t i = 0; i < parser.Get("id").AsArray().Size(); i++) {
                    if (parser.Get("id").AsArray().Get(i).GetString() == pkg) removedPackageAt = i;
                    else id.Push(ArrayElement((string)parser.Get("id").AsArray().Get(i).GetString(), -1));
                }
                for (size_t i = 0; i < parser.Get("version").AsArray().Size(); i++) if (i != removedPackageAt) version.Push(ArrayElement((string)parser.Get("version").AsArray().Get(i).GetString(), -1));
                for (size_t i = 0; i < parser.Get("build").AsArray().Size(); i++) if (i != removedPackageAt) build.Push(ArrayElement((string)parser.Get("build").AsArray().Get(i).GetString(), -1));
                for (size_t i = 0; i < parser.Get("name").AsArray().Size(); i++) if (i != removedPackageAt) name.Push(ArrayElement((string)parser.Get("name").AsArray().Get(i).GetString(), -1));
                writer.Push("id", (Array)id);
                writer.Push("name", (Array)name);
                writer.Push("version", (Array)version);
                writer.Push("build", (Array)build);
                writer.Write("/Apps/packages.tnb");
            }
            fP.close();
        }
        cout << "Removed " << removed << " packages!" << endl;
    } else if (arg == "add") {
        if (argc < 3) { cout << "Usage: " << argv[0] << " add <PACKAGES:id/name/filename>" << endl; return 0; }
        list<string> packagesToInstall;
        int installed = 0;
        for (int i = 2; i < argc; i++) packagesToInstall.push_back(string(argv[i]));
        for (string pkg : packagesToInstall) {
            ifstream f (pkg);
            bool fromUrl = false;
            if (!f.good()) fromUrl = true;
            f.close();
            installed++;
            if (!fromUrl) {
                string buffer;
                ifstream ifi (pkg);
                while (ifi) buffer += ifi.get();
                ifi.close();
                string codeForParser;
                string cache;
                string filesForParser;
                bool parsedInfo = false;
                for (size_t i = 0; i < buffer.size(); i++) {
                    if (i == buffer.size()-1) break;
                    if (!parsedInfo && buffer[i] == (char)126) { codeForParser = Desize(cache, 2); cache.clear(); parsedInfo = true; continue; }
                    cache += buffer[i];
                }
                filesForParser = cache;
                ofstream fi (".cachedCodeForParsing");
                fi << codeForParser;
                fi.close();
                TNBParser parsedParserOfTNB = TNBParser(".cachedCodeForParsing");
                remove(".cachedCodeForParsing");
                ofstream of (".cachedCodeForParsing");
                of << filesForParser;
                of.close();
                remove_all("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString());
                create_directory("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString());
                MoveFile(".cachedCodeForParsing", "/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/DECOMP.tgz");
                ofstream chksumOF ("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/checksum");
                chksumOF << parsedParserOfTNB.Get("checksum").GetString() << "  DECOMP.tgz";
                chksumOF.close();
                int isGood = system(("cd /etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + " && sha256sum -c checksum &> /dev/null").c_str());
                if (isGood != 0) {
                    cout << "Package '" << pkg << "' is not validated!" << endl;
                    remove_all("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString());
                    return 0;
                }
                create_directory("/Apps/" + parsedParserOfTNB.Get("package_id").GetString());
                remove("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/checksum");
                system(("cd /etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + 
                    " && tar -xf DECOMP.tgz && cp -r * /Apps/" + parsedParserOfTNB.Get("package_id").GetString()).c_str());
                remove("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/DECOMP.tgz");
                ifstream fP ("/Apps/packages.tnb");
                if (!fP.good()) {
                    TNBWriter writer;
                    Array id; id.Push(ArrayElement((string)parsedParserOfTNB.Get("package_id").GetString(), -1));
                    Array name; name.Push(ArrayElement((string)parsedParserOfTNB.Get("name").GetString(), -1));
                    Array build; build.Push(ArrayElement((string)parsedParserOfTNB.Get("build").GetString(), -1));
                    Array version; version.Push(ArrayElement((string)parsedParserOfTNB.Get("version").GetString(), -1));
                    writer.Push("id", (Array)id);
                    writer.Push("name", (Array)name);
                    writer.Push("version", (Array)version);
                    writer.Push("build", (Array)build);
                    writer.Write("/Apps/packages.tnb");
                } else {
                    TNBParser parser = TNBParser("/Apps/packages.tnb");
                    TNBWriter writer;
                    size_t skip;
                    Array id; for (size_t i = 0; i < parser.Get("id").AsArray().Size(); i++) {
                        if (parser.Get("id").AsArray().Get(i).GetString() != parsedParserOfTNB.Get("package_id").GetString()) 
                            id.Push(parser.Get("id").AsArray().Get(i)); 
                        else skip = i;
                    }
                    Array name; for (size_t i = 0; i < parser.Get("name").AsArray().Size(); i++) 
                        if (parser.Get("name").AsArray().Get(i).GetString() != parsedParserOfTNB.Get("name").GetString()) 
                            name.Push(parser.Get("name").AsArray().Get(i)); 
                    Array build; for (size_t i = 0; i < parser.Get("build").AsArray().Size(); i++) if (i != skip) build.Push(parser.Get("build").AsArray().Get(i)); 
                    Array version; for (size_t i = 0; i < parser.Get("version").AsArray().Size(); i++) if (i != skip) version.Push(parser.Get("version").AsArray().Get(i)); 
                    id.Push(ArrayElement((string)parsedParserOfTNB.Get("package_id").GetString(), -1));
                    name.Push(ArrayElement((string)parsedParserOfTNB.Get("name").GetString(), -1));
                    build.Push(ArrayElement((string)parsedParserOfTNB.Get("build").GetString(), -1));
                    version.Push(ArrayElement((string)parsedParserOfTNB.Get("version").GetString(), -1));
                    writer.Push("id", (Array)id);
                    writer.Push("name", (Array)name);
                    writer.Push("version", (Array)version);
                    writer.Push("build", (Array)build);
                    writer.Write("/Apps/packages.tnb");
                }
                fP.close();
            } else {
                ifstream fR ("/etc/Package/Repos");
                if (!fR.good()) {
                    TNBWriter writer;
                    writer.Push("repos", (Array)Array());
                    writer.Write("/etc/Package/Repos");
                    cout << "No repositories found! Setup default by using: " << argv[0] << " repo default" << endl;
                } else {
                    TNBParser parser = TNBParser("/etc/Package/Repos");
                    if (parser.Get("repos").AsArray().Size() == 0) cout << "No repositories found! Setup default by using: " << argv[0] << " repo default" << endl;
                    else {
                        string url;
                        bool found = false;
                        for (size_t i = 0; i < parser.Get("repos").AsArray().Size(); i++) {
                            string repo = parser.Get("repos").AsArray().Get(i).GetString(); 
                            curlpp::Cleanup myCleanup;
                            ofstream of (".repo.tnb");
                            of << curlpp::options::Url(repo + "/repo.tnb");
                            of.close();
                            TNBParser parserR = TNBParser(".repo.tnb");
                            for (size_t i = 0; i < parserR.Get("id").AsArray().Size(); i++) {
                                if (parserR.Get("id").AsArray().Get(i).GetString() == pkg ||
                                    parserR.Get("name").AsArray().Get(i).GetString() == pkg) { found = true; url = repo + "/pkgs/" + parserR.Get("url").AsArray().Get(i).GetString(); }
                            }
                        }
                        if (!found) { cout << "Package '" << pkg << "' not found in your repositories!" << endl; remove(".repo.tnb"); installed--; continue; }
                        else {
                            curlpp::Cleanup myCleanup;
                            ofstream of (".pkg");
                            of << curlpp::options::Url(url);
                            of.close();
                            string buffer;
                            ifstream ifi (".pkg");
                            while (ifi) buffer += ifi.get();
                            ifi.close();
                            string codeForParser;
                            string cache;
                            string filesForParser;
                            bool parsedInfo = false;
                            for (size_t i = 0; i < buffer.size(); i++) {
                                if (i == buffer.size()-1) break;
                                if (!parsedInfo && buffer[i] == (char)126) { codeForParser = Desize(cache, 2); cache.clear(); parsedInfo = true; continue; }
                                cache += buffer[i];
                            }
                            filesForParser = cache;
                            ofstream fi (".cachedCodeForParsing");
                            fi << codeForParser;
                            fi.close();
                            TNBParser parsedParserOfTNB = TNBParser(".cachedCodeForParsing");
                            remove(".cachedCodeForParsing");
                            ofstream ofi (".cachedCodeForParsing");
                            ofi << filesForParser;
                            ofi.close();
                            remove_all("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString());
                            create_directory("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString());
                            MoveFile(".cachedCodeForParsing", "/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/DECOMP.tgz");
                            ofstream chksumOF ("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/checksum");
                            chksumOF << parsedParserOfTNB.Get("checksum").GetString() << "  DECOMP.tgz";
                            chksumOF.close();
                            int isGood = system(("cd /etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + " && sha256sum -c checksum &> /dev/null").c_str());
                            if (isGood != 0) {
                                cout << "Package '" << pkg << "' is not validated!" << endl;
                                remove_all("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString());
                                return 0;
                            }
                            create_directory("/Apps/" + parsedParserOfTNB.Get("package_id").GetString());
                            remove("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/checksum");
                            system(("cd /etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + 
                                " && tar -xf DECOMP.tgz && cp -r * /Apps/" + parsedParserOfTNB.Get("package_id").GetString()).c_str());
                            remove("/etc/Package/Cache/Installer/" + parsedParserOfTNB.Get("package_id").GetString() + "/DECOMP.tgz");
                            ifstream fP ("/Apps/packages.tnb");
                            if (!fP.good()) {
                                TNBWriter writer;
                                Array id; id.Push(ArrayElement((string)parsedParserOfTNB.Get("package_id").GetString(), -1));
                                Array name; name.Push(ArrayElement((string)parsedParserOfTNB.Get("name").GetString(), -1));
                                Array build; build.Push(ArrayElement((string)parsedParserOfTNB.Get("build").GetString(), -1));
                                Array version; version.Push(ArrayElement((string)parsedParserOfTNB.Get("version").GetString(), -1));
                                writer.Push("id", (Array)id);
                                writer.Push("name", (Array)name);
                                writer.Push("version", (Array)version);
                                writer.Push("build", (Array)build);
                                writer.Write("/Apps/packages.tnb");
                            } else {
                                TNBParser parser = TNBParser("/Apps/packages.tnb");
                                TNBWriter writer;
                                size_t skip;
                                Array id; for (size_t i = 0; i < parser.Get("id").AsArray().Size(); i++) {
                                    if (parser.Get("id").AsArray().Get(i).GetString() != parsedParserOfTNB.Get("package_id").GetString()) 
                                        id.Push(parser.Get("id").AsArray().Get(i)); 
                                    else skip = i;
                                }
                                Array name; for (size_t i = 0; i < parser.Get("name").AsArray().Size(); i++) 
                                    if (parser.Get("name").AsArray().Get(i).GetString() != parsedParserOfTNB.Get("name").GetString()) 
                                        name.Push(parser.Get("name").AsArray().Get(i)); 
                                Array build; for (size_t i = 0; i < parser.Get("build").AsArray().Size(); i++) if (i != skip) build.Push(parser.Get("build").AsArray().Get(i)); 
                                Array version; for (size_t i = 0; i < parser.Get("version").AsArray().Size(); i++) if (i != skip) version.Push(parser.Get("version").AsArray().Get(i)); 
                                id.Push(ArrayElement((string)parsedParserOfTNB.Get("package_id").GetString(), -1));
                                name.Push(ArrayElement((string)parsedParserOfTNB.Get("name").GetString(), -1));
                                build.Push(ArrayElement((string)parsedParserOfTNB.Get("build").GetString(), -1));
                                version.Push(ArrayElement((string)parsedParserOfTNB.Get("version").GetString(), -1));
                                writer.Push("id", (Array)id);
                                writer.Push("name", (Array)name);
                                writer.Push("version", (Array)version);
                                writer.Push("build", (Array)build);
                                writer.Write("/Apps/packages.tnb");
                            }
                            fP.close();
                            remove(".pkg");
                        }
                    }
                }
                fR.close();
            }
            remove(".repo.tnb");
            cout << "Package '" << pkg << "' installed!" << endl;
        }
        cout << "Installed " << installed << " packages!" << endl;
    } else if (arg == "list-packages") {
        if (argc < 3) { cout << "Usage: " << argv[0] << " list-packages installed/available" << endl; return 0; }
        string typeOfPackages = argv[2];
        if (typeOfPackages == "installed") {
            ifstream f ("/Apps/packages.tnb");
            if (!f.good()) cout << "No packages installed!" << endl;
            else {
                TNBParser parser = TNBParser("/Apps/packages.tnb");
                if (parser.Get("id").AsArray().Size() == 0) cout << "No packages installed!" << endl;
                else {
                    for (size_t i = 0; i < parser.Get("id").AsArray().Size(); i++) {
                        string id = parser.Get("id").AsArray().Get(i).GetString();
                        string name = parser.Get("name").AsArray().Get(i).GetString();
                        string build = parser.Get("build").AsArray().Get(i).GetString();
                        string version = parser.Get("version").AsArray().Get(i).GetString();
                        cout << "PACKAGE ID=" << id << ", NAME=" << name << ", BUILD=" << build << ", VERSION=" << version << endl;
                    }
                }
            }
            f.close();
        } else cout << "SOON" << endl;
    } else if (arg == "version") {
        cout << "Package version: 1.0.0-SNAPSHOT" << endl;
        cout << "Package build: 1-stable" << endl;
    } else if (arg == "repo") {
        if (argc < 3) { cout << "Usage: " << argv[0] << " repo add/list/remove/default/new/insert-pkg/remove-pkg <REPOS> <PKG>" << endl; return 0; }
        string cmd = argv[2];
        if (cmd == "list") {
            ifstream f ("/etc/Package/Repos");
            if (!f.good()) {
                TNBWriter writer;
                writer.Push("repos", (Array)Array());
                writer.Write("/etc/Package/Repos");
                cout << "No repositories found! Setup default by using: " << argv[0] << " repo default" << endl;
            } else {
                TNBParser parser = TNBParser("/etc/Package/Repos");
                if (parser.Get("repos").AsArray().Size() == 0) cout << "No repositories found! Setup default by using: " << argv[0] << " repo default" << endl;
                else for (size_t i = 0; i < parser.Get("repos").AsArray().Size(); i++) cout << "REPOSITORY " << parser.Get("repos").AsArray().Get(i).GetString() << endl; 
            }
            f.close();
        } else if (cmd == "default") {
            string confirmation;
            cout << "Confirm rewriting repositories" << endl;
            cout << "[1] Proceed" << endl;
            cout << "[2] Exit" << endl;
            cout << "Your option? "; cin >> confirmation;
            if (confirmation != "1") { cout << "Exiting!" << endl; return 0; }
            TNBWriter writer;
            Array arr;
            arr.Push(ArrayElement((string)"https://raw.githubusercontent.com/theFFPS/theffps.github.io/main", -1));
            writer.Push("repos", (Array)arr);
            writer.Write("/etc/Package/Repos");
            cout << "Rewrote repositories with default!" << endl;
        } else if (cmd == "add") {
            if (argc < 4) { cout << "Usage: " << argv[0] << " repo add <REPOS>" << endl; return 0; }
            list<string> repos;
            for (int i = 3; i < argc; i++) repos.push_back(string(argv[i]));
            ifstream f ("/etc/Package/Repos");
            if (!f.good()) {
                TNBWriter writer;
                Array arr;
                for (string repo : repos) arr.Push(ArrayElement((string)repo, -1));
                writer.Push("repos", (Array)arr);
                writer.Write("/etc/Package/Repos");
            } else {
                TNBParser parser = TNBParser("/etc/Package/Repos");
                TNBWriter writer;
                Array arr;
                for (string repo : repos) arr.Push(ArrayElement((string)repo, -1));
                for (size_t i = 0; i < parser.Get("repos").AsArray().Size(); i++) arr.Push(parser.Get("repos").AsArray().Get(i));
                writer.Push("repos", (Array)arr);
                writer.Write("/etc/Package/Repos");
            }
            cout << "Finished!" << endl;
            f.close();
        } else if (cmd == "remove") {
            if (argc < 4) { cout << "Usage: " << argv[0] << " repo add <REPOS>" << endl; return 0; }
            list<string> repos;
            for (int i = 3; i < argc; i++) repos.push_back(string(argv[i]));
            ifstream f ("/etc/Package/Repos");
            if (!f.good()) {
                TNBWriter writer;
                writer.Push("repos", (Array)Array());
                writer.Write("/etc/Package/Repos");
            } else {
                TNBParser parser = TNBParser("/etc/Package/Repos");
                TNBWriter writer;
                Array arr;
                for (size_t i = 0; i < parser.Get("repos").AsArray().Size(); i++) {
                    bool found = false;
                    for (string repo : repos) if (repo == parser.Get("repos").AsArray().Get(i).GetString()) found = true;
                    if (!found) arr.Push(parser.Get("repos").AsArray().Get(i));
                }
                writer.Push("repos", (Array)arr);
                writer.Write("/etc/Package/Repos");
            }
            cout << "Finished!" << endl;
            f.close();
        } else if (cmd == "new") {
            TNBWriter writer;
            Array arrNAME; arrNAME.Push(ArrayElement((string)"_", -1));
            Array arrID; arrID.Push(ArrayElement((string)"_", -1));
            Array arrURL; arrURL.Push(ArrayElement((string)"_", -1));
            writer.Push("id", (Array)arrID);
            writer.Push("name", (Array)arrNAME);
            writer.Push("url", (Array)arrURL);
            writer.Write("repo.tnb");
            cout << "Copy file 'repo.tnb' to repository server!" << endl;
            cout << "To add package use '" << argv[0] << " repo insert-pkg <PKG>'" << endl;
        } else if (cmd == "insert-pkg") {
            if (argc < 4) { cout << "Usage: " << argv[0] << " repo insert-pkg <PKG>" << endl; return 0; }
            string pkg = argv[3];
            ifstream f ("repo.tnb");
            if (!f.good()) { cout << "Repository not created in this directory! Use '" << argv[0] << " repo new' to create it!" << endl; return 0; }
            f.close();
            TNBParser parser = TNBParser("repo.tnb");
            Array arrID;
            Array arrNAME;
            Array arrURL;
            string buffer;
            ifstream ifi (pkg);
            while (ifi) buffer += ifi.get();
            ifi.close();
            string codeForParser;
            string cache;
            bool parsedInfo = false;
            for (size_t i = 0; i < buffer.size(); i++) {
                if (i == buffer.size()-1) break;
                if (!parsedInfo && buffer[i] == (char)126) { codeForParser = Desize(cache, 2); cache.clear(); parsedInfo = true; continue; }
                cache += buffer[i];
            }
            ofstream fi (".cachedCodeForParsing");
            fi << codeForParser;
            fi.close();
            TNBParser parsedParserOfTNB = TNBParser(".cachedCodeForParsing");
            remove(".cachedCodeForParsing");
            for (size_t i = 0; i < parser.Get("id").AsArray().Size(); i++) {
                if (parser.Get("id").AsArray().Get(i).GetString() == "_") continue;
                if (parser.Get("id").AsArray().Get(i).GetString() != parsedParserOfTNB.Get("package_id").GetString()) arrID.Push(parser.Get("id").AsArray().Get(i));
            }
            for (size_t i = 0; i < parser.Get("name").AsArray().Size(); i++) {
                if (parser.Get("url").AsArray().Get(i).GetString() == "_") continue;
                if (parser.Get("name").AsArray().Get(i).GetString() != parsedParserOfTNB.Get("name").GetString()) arrNAME.Push(parser.Get("name").AsArray().Get(i));
            }
            for (size_t i = 0; i < parser.Get("url").AsArray().Size(); i++) {
                if (parser.Get("url").AsArray().Get(i).GetString() == "_") continue;
                if (parser.Get("url").AsArray().Get(i).GetString() != pkg) arrURL.Push(parser.Get("url").AsArray().Get(i));
            }
            TNBWriter writer;
            arrID.Push(ArrayElement((string)parsedParserOfTNB.Get("package_id").GetString(), -1));
            arrURL.Push(ArrayElement((string)pkg, -1));
            arrNAME.Push(ArrayElement((string)parsedParserOfTNB.Get("name").GetString(), -1));
            writer.Push("id", (Array)arrID);
            writer.Push("name", (Array)arrNAME);
            writer.Push("url", (Array)arrURL);
            writer.Write("repo.tnb");
            cout << "Added!" << endl;
            cout << "To remove package use '" << argv[0] << " repo remove-pkg <PKG>'" << endl;
        } else if (cmd == "remove-pkg") {
            if (argc < 4) { cout << "Usage: " << argv[0] << " repo remove-pkg <PKG>" << endl; return 0; }
            string pkg = argv[3];
            ifstream f ("repo.tnb");
            if (!f.good()) { cout << "Repository not created in this directory! Use '" << argv[0] << " repo new' to create it!" << endl; return 0; }
            f.close();
            TNBParser parser = TNBParser("repo.tnb");
            Array arrID;
            Array arrNAME;
            Array arrURL;
            size_t skipI = 0;
            string buffer;
            ifstream ifi (pkg);
            while (ifi) buffer += ifi.get();
            ifi.close();
            string codeForParser;
            string cache;
            bool parsedInfo = false;
            for (size_t i = 0; i < buffer.size(); i++) {
                if (i == buffer.size()-1) break;
                if (!parsedInfo && buffer[i] == (char)126) { codeForParser = Desize(cache, 2); cache.clear(); parsedInfo = true; continue; }
                cache += buffer[i];
            }
            ofstream fi (".cachedCodeForParsing");
            fi << codeForParser;
            fi.close();
            TNBParser parsedParserOfTNB = TNBParser(".cachedCodeForParsing");
            remove(".cachedCodeForParsing");
            for (size_t i = 0; i < parser.Get("id").AsArray().Size(); i++) {
                if (parser.Get("id").AsArray().Get(i).GetString() == parsedParserOfTNB.Get("project_id").GetString()) skipI = i;
                else arrID.Push(parser.Get("id").AsArray().Get(i));
            }
            for (size_t i = 0; i < parser.Get("name").AsArray().Size(); i++) if (i != skipI) arrNAME.Push(parser.Get("name").AsArray().Get(i));
            for (size_t i = 0; i < parser.Get("url").AsArray().Size(); i++) if (i != skipI) arrURL.Push(parser.Get("url").AsArray().Get(i));
            TNBWriter writer;
            if (arrID.Size() == 0) {
                arrID.Push(ArrayElement((string)"_", -1));
                arrNAME.Push(ArrayElement((string)"_", -1));
                arrURL.Push(ArrayElement((string)"_", -1));
            }
            writer.Push("id", (Array)arrID);
            writer.Push("name", (Array)arrNAME);
            writer.Push("url", (Array)arrURL);
            writer.Write("repo.tnb");
            cout << "Removed!" << endl;
            cout << "To add package use '" << argv[0] << " repo insert-pkg <PKG>'" << endl;
        }
    } else {
        cout << "project <DIR> set/get/get-all/available-fields <FIELD> <VALUE>          Manage project" << endl;
        cout << "package <DIR>                                                           Create package file from project located in <DIR>" << endl;
        cout << "help                                                                    This message" << endl;
        cout << "version                                                                 Print version" << endl;
        cout << "add <PACKAGES:id/name/filename>                                         Install packages" << endl;
        cout << "remove <PACKAGES:id>                                                    Remove packages" << endl;
        cout << "repo add/list/remove/default/new/insert-pkg/remove-pkg <REPOS> <PKG>    Manage repositories" << endl;
        cout << "list-packages installed/available                                       List packages" << endl;
    }
    return 0;
}
