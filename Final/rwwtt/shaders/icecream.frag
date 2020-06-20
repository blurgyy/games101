#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(binding = 0) uniform UniformBufferObject{
    vec2 res;
    float time;
    vec2 mouse;
} passedInfo;
layout(location = 0) out vec4 outColor;
const int MAXSTEPS = 128;
const float MAXDIST = 1e8;
const float EPS = 1e-5;
const float PI = acos(-1.0);
const float TWOPI = 2*acos(-1.0);
const int AA = 2;

#define MAT_CONE    0
#define MAT_CREAM   1
#define MAT_GROUND  2
#define MAT_CANDY_1 3
#define MAT_CANDY_2 4
#define MAT_CANDY_3 5
#define MAT_CANDY_4 6
#define MAT_CANDY_5 7
#define MAT_CANDY_6 MAT_CANDY_4
#define MAT_CANDY_7 MAT_CANDY_1
#define MAT_CANDY_8 MAT_CANDY_2
#define MAT_CANDY_9 MAT_CANDY_5
#define MAT_CANDY_0 MAT_CANDY_3
#define CANDY_RN    0.65

struct Material{
    vec3 kd;            // color
    float rn;           // roughness
};
const Material materials[] =  {
    Material( // cone
        vec3(0.8627, 0.6510, 0.3255),
        1.00
    ),
    Material( // cream
        vec3(0.9255, 0.9294, 0.8510),
        0.25
    ),
    Material( // ground
        vec3(0.9451, 0.9451, 0.9451),
        0.90
    ),
    Material( // candy1
        vec3(0.9451, 0.1324, 0.1216),
        CANDY_RN
    ),
    Material( // candy2
        vec3(0.1397, 0.9288, 0.0943),
        CANDY_RN
    ),
    Material( // candy3
        vec3(0.1307, 0.3043, 0.9451),
        CANDY_RN
    ),
    Material( // candy4
        vec3(0.9670, 0.9451, 0.0642),
        CANDY_RN
    ),
    Material( // candy5
        vec3(0.8431, 0.3485, 0.5084),
        CANDY_RN
    )
};

// ------------------------------------------------
// from: https://www.shadertoy.com/view/4tByz3
vec3 rotateY( in vec3 p, float t ){
    float co = cos(t);
    float si = sin(t);
    p.xz = mat2(co,-si,si,co)*p.xz;
    return p;
}
vec3 rotateX( in vec3 p, float t ){
    float co = cos(t);
    float si = sin(t);
    p.yz = mat2(co,-si,si,co)*p.yz;
    return p;
}
vec3 rotateZ( in vec3 p, float t ){
    float co = cos(t);
    float si = sin(t);
    p.xy = mat2(co,-si,si,co)*p.xy;
    return p;
}
// smooth min function for blending distance fields
// from: https://iquilezles.org/www/articles/smin/smin.htm
float smin( float a, float b, float k ){
    // polynomial smooth min (k = 0.1);
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}
// ------------------------------------------------

float dot2(vec2 x){return dot(x, x);}
float dot2(vec3 x){return dot(x, x);}
mat3 rot3x(float ang){
    float co = cos(ang), si = sin(ang);
    return mat3(1, 0, 0,
                0, co, -si,
                0, si, co);
}
mat3 rot3y(float ang){
    float co = cos(ang), si = sin(ang);
    return mat3(co, 0, -si,
                0, 1, 0,
                si, 0, co);
}
mat3 rot3z(float ang){
    float co = cos(ang), si = sin(ang);
    return mat3(co, -si, 0,
                si, co, 0,
                0, 0, 1);
}
mat3 rot3(float x, float y, float z){
    return rot3z(z) * rot3y(y) * rot3x(x);
}

// ------------------------------------------------

float sdPlane(vec3 p){
    return -p.y;
}
float sdCappedRoundCone(vec3 p, float h, float r1, float r2, float corner){
    // reference: https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
    vec2 q = vec2(length(p.xz), -p.y);
    vec2 b = vec2(r2, -h);
    vec2 m = vec2(r1-r2, 2.0*h);
    vec2 hor = vec2(q.x-min(q.x, (q.y<0.0)?r2:r1), abs(q.y)-h);
    vec2 ver = b-q + m * clamp(dot(q-b, m)/dot2(m), 0, 1);
    // check if p is inside the cone
    float s = (ver.x>0.0 && hor.y<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(dot2(hor),dot2(ver)) );
}
float sdBowl(vec3 p, float r, float r1, float r2){
    float h1 = sqrt(r*r - r1*r1);
    float h2 = sqrt(r*r - r2*r2);
    float dist = MAXDIST;
    vec2 q = vec2(length(p.xz), -p.y);

    if(q.y > -h1 && q.x < r1){
        dist = q.y + h1;
    } else if(q.y * r1 > -h1 * q.x){
        dist = length(q - vec2(r1, -h1));
    } else if(q.y < -h2 && q.x < r2){
        dist = -h2 - q.y;
    } else {
        dist = min(length(q-vec2(r2, -h2)), length(q)-r);
    }
    return dist;
}
float sdTorus(vec3 p, vec2 t){
    vec2 q = vec2(length(p.xz)-t.x,p.y);
    return length(q)-t.y;
}
float sdRoundCone( vec3 p, float r, float h, float corner ){
    h -= 2*corner; r -= corner;
    return sdCappedRoundCone(p-vec3(0,-h/2,0), h/2, EPS, r, 0) - corner;
}
float sdStick(vec3 p, vec3 a, vec3 b, float r){
    vec3 ab = b - a;
    vec3 ap = p - a;
    return length(clamp(dot(ab, ap)/dot2(ab), 0, 1) * ab - ap) - r;
}
float sdRoundBox(vec3 p, vec4 shape){
    float corner = shape.w;
    shape.xyz = max(shape.xyz - corner, 0);
    vec3 q = abs(p) - shape.xyz;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - corner;
}

// ------------------------------------------------

float mapCream(vec3 p, vec3 bottom, vec3 offset,
               int rep, float angx, float angy, float r, float h){
    // vec3 offset (in which offset.y is the radius of the round corner)
    // int rep, float angx, float angy, float r, float h
    float cosx = cos(angx), sinx = sin(angx);
    float cosy = cos(angy), siny = sin(angy);
    const mat3 rotx = mat3(
        1,  0,       0,
        0, cosx, -sinx,
        0, sinx,  cosx
    );
    const mat3 roty = mat3(
        cosy, 0, -siny,
           0, 1,     0,
        siny, 0,  cosy
    );
    mat3 invrot = inverse(roty * rotx);
    float corner = abs(offset.y);
    float dist = MAXDIST;
    for(int i = 0; i < rep; ++ i){
        vec3 q = rotateY(p, TWOPI*i/rep);
        q = q - bottom - offset;
        q = invrot * q;
        dist = smin(dist, sdRoundCone(q, r, h, corner), 0.01);
    }
    return dist;
}

float mapBars(vec3 p, int rep, float r, float r_bot, float r_top, float h){
    // float l = sqrt(h*h + (r_bot-r_top)*(r_bot-r_top));
    float dist = MAXDIST;
    for(int i = 0; i < rep; ++ i){
        float ang = 1.0*TWOPI*i/rep;
        float co = cos(ang);
        float si = sin(ang);
        vec3 bot = vec3(r_bot*co, 0, r_bot*si);
        vec3 top = vec3(r_top*co, -h, r_top*si);
        dist = min(dist, sdStick(p, bot, top, r));
    }
    return dist;
}

// ------------------------------------------------

vec2 mapCandy(vec3 p, vec2 last){
    const vec4 shape = vec4(0.02, 0.03, 0.01, 0.007);
    vec3 cent1 = vec3(0.12, -1, 16);
    vec3 cent2 = vec3(0.06, -1.2, -0.08);
    vec3 cent5 = vec3(-0.06, -1.1, -0.11);
    vec3 cent6 = vec3(0.168, -0.95, -0.156);
    vec3 cent7 = vec3(0.01, -0.94, 0.19);
    vec3 cent3 = vec3(-0.16, -0.9, 0.11);
    vec3 cent4 = vec3(-0.2, -0.84, -0.2);
    // 3x floating candies
    vec3 cent8 = vec3(0.5, -1, 0.866)    + vec3(0, 0.07*sin(0.15*passedInfo.time),  0);
    vec3 cent9 = vec3(-1.0, -0.8, 0.0)   + vec3(0, 0.05*cos(0.13*passedInfo.time),  0);
    vec3 cent0 = vec3(0.5, -1.2, -0.866) + vec3(0, 0.06*sin(0.17*passedInfo.time), 0);

    vec2 ret = last;
    float dist = MAXDIST;
    vec3 q = p;
    // candy #1
    q = inverse(rot3(12, 42, 61))*(p-cent1);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_1;
    }
    // candy #2
    q = inverse(rot3(56, 21, 30))*(p-cent2);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_2;
    }
    // candy #3
    q = inverse(rot3(36, 50, 28))*(p-cent3);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_3;
    }
    // candy #4
    q = inverse(rot3(26, 75, 32))*(p-cent4);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_4;
    }
    // candy #5
    q = inverse(rot3(34, 49, 29))*(p-cent5);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_5;
    }
    // candy #6
    q = inverse(rot3(12, 28, 54))*(p-cent6);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_6;
    }
    // candy #7
    q = inverse(rot3(75, 83, 60))*(p-cent7);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_7;
    }
    float delta_rot = 0.07*passedInfo.time+0.05*sin(passedInfo.time);
    // floating candy #8
    q = inverse(rot3(76+delta_rot, 29, 86))*(p-cent8);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_8;
    }
    // floating candy #9
    q = inverse(rot3(73, 46+delta_rot, 14))*(p-cent9);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_9;
    }
    // floating candy #0
    q = inverse(rot3(91, 63, 41+delta_rot))*(p-cent0);
    dist = min(dist, sdRoundBox(q, shape));
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CANDY_0;
    }
    return ret;
}

vec2 mapHead(vec3 p, vec3 bottom, vec2 last){
    vec3 q_top = rotateY(p, -p.y * 6);
    vec3 q_bot = rotateY(p, -p.y * 5);
    // bottom cream
    vec3 offset_bot = vec3(0.1, -0.03, 0.3);
    int rep_bot = 7;
    float r_bot = 0.16,
          h_bot = 0.40,
          x_bot = -1,
          y_bot = PI/10;
    // top cream
    vec3 offset_top = vec3(0.06, -0.03, 0.3);
    int rep_top = 5;
    float r_top = 0.10,
          h_top = 0.60,
          x_top = -0.33,
          y_top = -PI/3;

    float dist = mapCream(q_bot, bottom, offset_bot, rep_bot, x_bot, y_bot, r_bot, h_bot);
    bottom += -h_bot * cos(x_bot) + vec3(0, 0.05, 0);
    dist = smin(dist, mapCream(q_top, bottom, offset_top, rep_top, x_top, y_top, r_top, h_top), 0.05);
    vec2 ret = last;
    if(dist < ret.x){
        ret.x = smin(ret.x, dist, 0.01);
        ret.y = MAT_CREAM;
    }
    return ret;
}

vec2 mapCone(vec3 p, out vec3 newbase, vec2 last){
    // cone1
    vec3 cone1_cent = vec3(0, -0.23, 0);
    vec3 cone1_top = 2*cone1_cent;
    vec3 cone1_bot = vec3(0, 0, 0);
    float cone1_r_top = 0.25,
          cone1_r_bot = 0.2,
          cone1_halfh = abs(cone1_cent.y);
    // cone2
    vec3 cone2_cent = vec3(0, -0.02, 0);
    vec3 cone2_top = 2*cone2_cent;
    vec3 cone2_bot = vec3(0, 0, 0);
    float cone2_r_top = cone1_r_top * 0.95,
          cone2_r_bot = cone1_r_top * 1.07,
          cone2_halfh = abs(cone2_cent.y);
    cone2_cent += cone1_top;
    cone2_top += cone1_top;
    cone2_bot += cone1_top;
    // bowl
    vec3 bowl_cent = vec3(0);
    float bowl_r = cone1_r_top * 1.3,
          bowl_r_top = bowl_r * 0.99,
          bowl_r_bot = bowl_r * 0.78,
          bowl_h_top = sqrt(bowl_r*bowl_r - bowl_r_top*bowl_r_top),
          bowl_h_bot = sqrt(bowl_r*bowl_r - bowl_r_bot*bowl_r_bot),
          bowl_h = abs(bowl_h_bot - bowl_h_top);
    bowl_cent += cone2_top - vec3(0,bowl_h_bot,0);
    vec3 bowl_top = bowl_cent + vec3(0,bowl_h_top,0);
    vec3 bowl_bot = bowl_cent + vec3(0,bowl_h_bot,0);
    // rings on cone1
    float ring_wid = 0.0075;
    vec3 ring1_cent = mix(cone1_top, cone1_bot, 0.25); vec2 ring1 = vec2(mix(cone1_r_top, cone1_r_bot, 0.25), ring_wid);
    vec3 ring2_cent = mix(cone1_top, cone1_bot, 0.50); vec2 ring2 = vec2(mix(cone1_r_top, cone1_r_bot, 0.50), ring_wid);
    vec3 ring3_cent = mix(cone1_top, cone1_bot, 0.75); vec2 ring3 = vec2(mix(cone1_r_top, cone1_r_bot, 0.75), ring_wid);
    // bars on cone1
    float bar_wid = 0.01;

    float bars = mapBars(p, 7, bar_wid, cone1_r_bot, cone1_r_top, 2*cone1_halfh);
    float rings = sdTorus(p-ring1_cent, ring1);
    rings = min(rings, sdTorus(p-ring2_cent, ring2));
    rings = min(rings, sdTorus(p-ring3_cent, ring3));
    rings = smin(rings, bars, (ring_wid+bar_wid)/2);
    float cones = sdCappedRoundCone(p-cone1_cent, cone1_halfh, cone1_r_top, cone1_r_bot, ring_wid);
    cones = min(cones, sdCappedRoundCone(p-cone2_cent, cone2_halfh, cone2_r_top, cone2_r_bot, ring_wid));
    cones = min(cones, sdBowl(p-bowl_cent, bowl_r, bowl_r_top, bowl_r_bot));
    float dist = smin(rings, cones, ring_wid);
    vec2 ret = last;
    if(dist < ret.x){
        ret.x = dist;
        ret.y = MAT_CONE;
    }
    newbase = bowl_top;
    return ret;
}

vec2 map(vec3 p){
    vec2 ret = vec2(sdPlane(p), MAT_GROUND);
    vec3 cone_top = vec3(0);
    ret = mapCone(p, cone_top, ret);
    ret = mapHead(p, cone_top, ret);
    ret = mapCandy(p, ret);
    return ret;
}

vec3 getNormal(vec3 p){
    vec2 eps = vec2(EPS, 0);
    vec3 n = vec3(
    	map(p + eps.xyy).x - map(p - eps.xyy).x,
        map(p + eps.yxy).x - map(p - eps.yxy).x,
        map(p + eps.yyx).x - map(p - eps.yyx).x);
    return normalize(n);
}

vec2 castRay(vec3 ro, vec3 rd){
    float tmin = 1.0;
    float tmax = MAXDIST;

    vec2 ret = vec2(tmin, -1.0);
    for(int i = 0; i < MAXSTEPS; ++ i){
        if(ret.x > tmax){
            break;
        }
        float adaptive_eps = EPS * ret.x; // !
        vec3 pos = ro + ret.x * rd;
        vec2 delta = map(pos);
        if(delta.x < adaptive_eps){
            ret.y = delta.y;
            break;
        }
        ret.x += delta.x;
        ret.y = delta.y;
    }
    if(ret.x > tmax){
        ret.x = -1.0;
        ret.y = -1.0;
    }
    return ret;
}

float softShadow(vec3 ro, vec3 rd, float tmin, float tmax){
    // use tmin > 0 to prevent rays from stopping without leaving local area
    float ret = 1.0;
    float t = tmin;
    float prev_h = 1e20;
    for(int i = 0; i < MAXSTEPS; ++ i){
        vec3 pos = ro + t * rd;
        float h = map(pos).x;
        // ret = min(ret, 10 * h / t);
        if(h < EPS){
            return 0;
        }
        float y = h*h/(2*prev_h);
        float d = sqrt(h*h - y*y);
        ret = min( ret, 32*d/max(t-y,0) );
        prev_h = h;
        t += h;
        if(t > tmax || ret < EPS){
            break;
        }
    }
    return clamp(ret, 0, 1);
}

float ambientOcc(vec3 p, vec3 n){
    float occ = 0;
    float scalar = 1;
    for(int i = 0; i < 5; ++ i){
        float h = EPS + 0.15 * i;
        float d = map(p + h*n).x;
        occ += (h-d) * scalar;
        scalar *= 0.5;
    }
    return clamp(1.0 - 1.5 * occ, 0, 1);
}

// rendering process, returns color
vec3 render(vec3 ro, vec3 rd){
    vec3 ret_color = vec3(0);

    vec2 point = castRay(ro, rd);
    float t = point.x;
    int mat_id = int(point.y + 0.5);

    if(t > -0.5){
        vec3 p = ro + t * rd;
        vec3 n = getNormal(p);

        // vec3 kd = vec3(0.3);
        vec3 kd = materials[mat_id].kd;
        float pn = exp2(10*(1-materials[mat_id].rn));

        // vec3 l = normalize(vec3(-4, -3, 1)); // parallel light rays
        vec3 l = normalize(vec3(-16, -12, 4)-p);
        vec3 h = normalize(l-rd);

        vec3 light = vec3(1.0, 0.7, 0.5)/1.2;
        float diffuse = clamp(dot(n, l), 0, 1) * softShadow(p, l, 0.07, 5.0);
        float specular = pow(clamp(dot(h, n), 0, 1), pn) * diffuse;

        ret_color = (kd*diffuse + specular) * light;

        vec3 ambientLight = vec3(0.2, 0.1, 0.0);
        float occ = ambientOcc(p, n);
        float AO = clamp(0.5 + 0.5 * n.y, 0, 1);

        ret_color += kd * occ * AO * ambientLight;

        // fog
        ret_color *= exp( -0.0005*t*t*t );
    }

    return clamp(ret_color, 0, 1);
}

// ----------------------------------------------------

mat3 setCamera( in vec3 ro, in vec3 ta, float cr ){
	vec3 cw = normalize(ta-ro);
	vec3 cp = vec3(sin(cr), cos(cr),0.0);
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, cw );
}

// ----------------------------------------------------

void main(){
    vec2 uv = (gl_FragCoord.xy - .5 * passedInfo.res.xy) / passedInfo.res.y;
    vec3 color = vec3(0.);

    // vec3 ro = vec3(0.0, -1.5, 2-4*int(mod(passedInfo.time, 2) < 1));
    // vec3 ro = vec3(-2, -1.5, 0.5);
    // vec3 ro = vec3(3*sin(0.7*length(passedInfo.mouse)*0.01),
    //                -1.5 + 0.3*sin(0.5*length(passedInfo.mouse)*0.01),
    //                2*cos(0.7*length(passedInfo.mouse)*0.01));
    vec3 ro = vec3(3*sin(0.1*passedInfo.time),
                   -1.5 + 0.5*sin(0.005*passedInfo.time),
                   2*cos(0.1*passedInfo.time));
    vec3 ta = vec3(0, -0.5, 0);
    mat3 camRot = setCamera( ro, ta, 0.0 );
    // vec3 rd = camRot * normalize(vec3(uv.x, uv.y, 1));

    // super sampling anti aliasing
    for(int i = 0; i < AA; ++ i){
        for(int j = 0; j < AA; ++ j){
            vec2 px = uv.xy + vec2(i, j)/AA/passedInfo.res.y;
            vec3 rd = camRot * normalize(vec3(px.xy, 1));
            color += render(ro, rd);
        }
    }
    color /= AA * AA;

    // color = render(ro, rd);
    // float t = castRay(ro, rd).x;
    // color = getNormal(ro + t * rd);
    outColor = vec4(color.xyz, 1.);
}
