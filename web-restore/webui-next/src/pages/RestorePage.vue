<script setup lang="ts">
import {ref} from "vue";

const catalog = ref('')
</script>

<template>
  <div class="row">
    <div class="bg-amber">
      <q-card>
        <q-card-section>
          <q-form class="q-gutter-md">
            <q-input filled
                     v-model="catalog"
                     hint="select your catalog"
            >

            </q-input>

          </q-form>
        </q-card-section>

        <q-card-section class="q-pt-none">
          Text
        </q-card-section>
      </q-card>

    </div>
    <div class="col-grow bg-blue-3">rechts</div>
  </div>
</template>

<style scoped>

</style>
