Import("env")

def build_web_app(source, target, env):
    env.Execute("cd src/web_app && npm install && npm run build")

env.AddPreAction("${BUILD_DIR}/src/EspDmxNode.cpp.o", build_web_app)